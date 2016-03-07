/*
 * BaseConnection.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE

#include <memory>
#include "BaseMsg.h"
#include "BaseConnection.h"
#include "ext/nmdutil/etcutil.h"
#include "ext/nmdutil/netutil.h"
#include "flog.h"

using namespace edft;
using namespace std;

enum {
	FP_CNN=0,
	FP_SVR=7,
};


#define FSET_CNN() BIT_SET(mStatusFlag, FP_CNN)
#define FSET_DISCNN() BIT_RESET(mStatusFlag, FP_CNN)
#define FGET_CNN() BIT_TEST(mStatusFlag, FP_CNN)
#define FSET_SVR() BIT_SET(mStatusFlag, FP_SVR)
#define FGET_SVR() BIT_TEST(mStatusFlag, FP_SVR)



namespace cahttp {

BaseConnection::BaseConnection() {
	mBuf = nullptr;
	mBufSize = 2048;
	mStatusFlag = 0;
	mHandleSeed=0;
}

BaseConnection::~BaseConnection() {
	alv("dest basecnn, ptr=%x", (long)this);
	close();
	if(mBuf) {
		delete[] mBuf;
	}
}


int BaseConnection::connect(uint32_t ip, int port, int timeout) {
	mSvrIp = ip;
	mSvrPort = port;
	if(!mBuf) {
		mBuf = new char[mBufSize];
		assert(mBuf);
	}

	init_sock(false, 0);

	mCnnTimer.setOnListener([this, ip, port]() {
		alw("*** connecting timeout... ip=%s, port=%d", cahttpu::Ip2CStr(ip), port);
		mCnnTimer.kill();
		mSocket.close();
		FSET_DISCNN();
		procClosed();
	});
	mCnnTimer.set(timeout);
	return mSocket.connect(ip, port);

}

SR BaseConnection::send(uint32_t handle, const char* buf, size_t len) {
	if(FGET_CNN()==0) {
		ald("*** not yet connected");
		return SR::eNext;
	}
	if(mTxChList.empty()) {
		assert(0);
		return SR::eFail;
	}
	if( mTxChList.front().handle != handle) {
		return SR::eNext;
	}
	if(mSocket.isWritable()) {
		auto wret = mSocket.sendPacket(buf, len);
		if(wret==edft::SEND_OK) return SR::eOk;
		else if(wret == edft::SEND_PENDING) return SR::ePending;
		else {
			ald("*** send fail in writable state, ret=%d", wret);
			return SR::eFail;
		}
	} else {
		ald("*** not writable");
		return SR::eNext;
	}
}

void BaseConnection::close() {
	if(mSocket.getFd()>=0) {
		ald("close socket, fd=%d, cnnptr=%x", mSocket.getFd(), (long)this);
		mSocket.close();
	}

	mCnnTimer.kill();
	mLocalEvent.close();
	mSvrIp = 0;
	mSvrPort = 0;
}

void BaseConnection::reserveWrite() {
	mSocket.reserveWrite();
}


int BaseConnection::procRead() {
	assert(mSocket.getFd()>0);
	alv("proc read, fd=%d, cnnptr=%x", mSocket.getFd(), (long)this);
	auto rcnt = mSocket.recvPacket(mBuf, mBufSize);
	if(rcnt>0) {
		auto ccnt = mMsgFrame.feedPacket(mBuf, rcnt);
		ald("consumed cnt in parser, cnt=%d", ccnt);
		alv("packet data:\n|%s|", string(mBuf, rcnt));
		if(ccnt) {
//			assert(ccnt == rcnt);
			int bexit=0;
			for(;!bexit;) {
				auto fetch_status = mMsgFrame.status();
				if(fetch_status == mMsgFrame.FS_HDR) {
					mRecvMsg.reset( new BaseMsg );
					mMsgFrame.fetchMsg( (*mRecvMsg) );
					if(mRxChList.size()) {
						bexit = mRxChList.front().lis(CH_MSG);
					} else {
						assert(mDefRxLis);
						bexit = mDefRxLis(CH_MSG);
					}
				} else if(fetch_status == mMsgFrame.FS_DATA) {
					mRecvData.clear();
					mMsgFrame.fetchData(mRecvData);
					if(mRxChList.size()) {
						bexit = mRxChList.front().lis(CH_DATA);
					} else {
						assert(mDefRxLis);
						bexit = mDefRxLis(CH_DATA);
					}
				} else if(fetch_status == mMsgFrame.FS_NONE) {
					break;
				}
			}
		} else {
			alw("*** http msg parser error: %s", mMsgFrame.getParserErrorDesp());
			assert(0);
			return -2;
		}
	} else {
		//ald("*** no read data");
	}
	return 0;
}



int BaseConnection::openServer(int fd) {
	FSET_SVR();
	init_sock(true, fd);
	return 0;
}


void BaseConnection::init_sock(bool svr, int fd) {
	if(!mBuf) {
		mBuf = new char[mBufSize];
		assert(mBuf);
	}
	mMsgFrame.init(svr);
	if(svr) {
		mSocket.openChild(fd);
		FSET_CNN();
	}

	mSocket.setOnListener([this](int event) {
			if(event == NETEV_CONNECTED) {
				ald("sock connected");
				mCnnTimer.kill();
				FSET_CNN();
				procWritable();
			} else if(event == NETEV_DISCONNECTED) {
				ald("*** sock disconnected...");
				mCnnTimer.kill();
				mSocket.close();
				FSET_DISCNN();
				procClosed();
				OnDisconnected();
			} else if(event == NETEV_READABLE) {
				alv("sock readble");
				procRead();
			} else if(event == NETEV_WRITABLE) {
				alv("sock writable");
				procWritable();
			}
	});

	if(mLocalEvent.getFd()<=0) {
		mLocalEvent.open(1000, [this](EdLocalEvent::Event &evt){
			ald("cnn local event, id=%d", evt.evt_id);
			if(evt.evt_id == 0) {
				mDummyChannels.clear();
				procClosed();
				OnIdle();
			}
		});
	}
}


uint32_t BaseConnection::openTxCh(ChLis lis) {
	if(++mHandleSeed==0) mHandleSeed++;
	mTxChList.emplace_back();
	auto &c = mTxChList.back();
	c.handle = mHandleSeed;
	c.lis = lis;
	return mHandleSeed;
}

uint32_t BaseConnection::openRxCh(ChLis lis) {
	mCnnTimer.kill();
	if(++mHandleSeed==0) mHandleSeed++;
	_chlis *c;
	mRxChList.emplace_back();
	c = &(mRxChList.back());
	c->handle = mHandleSeed;
	c->lis = lis;
	return mHandleSeed;
}

void BaseConnection::endTxCh(uint32_t h) {
	mCnnTimer.kill();
	if(mTxChList.size()>0 && mTxChList.front().handle==h) {
		mTxChList.pop_front();
		if(mTxChList.size()) {
			reserveWrite();
//			procWritable();
		}

		if(mTxChList.empty() && mRxChList.empty()){
			startIdleTimer();
		}
	} else {
		assert(mTxChList.size()>0 && mTxChList.front().handle==h);
	}
}

void BaseConnection::endRxCh(uint32_t h) {
	ald("end rx ch=%d", h);
	if(mRxChList.size()>0 && mRxChList.front().handle == h) {
		mRxChList.pop_front();
		if(mTxChList.empty() && mRxChList.empty()){
			startIdleTimer();
		}
	} else {
		assert(mRxChList.size()>0 && mRxChList.front().handle==h);
	}
#if 0
	for(auto itr=mRxChList.begin();itr != mRxChList.end(); itr++) {
		if(itr->handle == h) {
			mRxChList.erase(itr);
			break;
		}
	}
#endif
}


int BaseConnection::procWritable() {
	uint32_t h=0;
	for(;mTxChList.size()>0;) {
		auto &c = mTxChList.front();
		if(c.handle == h) {
			// if tx channel owner is the same as previous one, exit loop
			break;
		}
		h = c.handle;
		c.lis(CH_WRITABLE);
	}
	return 0;
}

void BaseConnection::removeRxChannel(uint32_t h) {
	for(auto itr=mRxChList.begin(); itr != mRxChList.end(); itr++) {
		if(itr->handle == h) {
			mRxChList.erase(itr);
		}
	}
}

void BaseConnection::removeTxChannel(uint32_t h) {
	for(auto itr=mTxChList.begin(); itr != mTxChList.end(); itr++) {
		if(itr->handle == h) {
			mTxChList.erase(itr);
		}
	}
}

int BaseConnection::procClosed() {
	std::list<_chlis> dummy;
	if(mRxChList.size()) {
		dummy = move(mRxChList);
		assert(mRxChList.empty());
		for(auto &c : dummy) {
			c.lis(CH_CLOSED);
		}
		dummy.clear();
	}
	if(mTxChList.size()) {
		dummy = move(mTxChList);
		assert(mTxChList.empty());
		for(auto &c : dummy) {
			c.lis(CH_CLOSED);
		}
		dummy.clear();
	}

	if(mDefRxLis) mDefRxLis(CH_CLOSED);
	return 0;
}

void BaseConnection::OnDisconnected() {
//	mCnnTimer.kill();
//	mSocket.close();
//	mSvrIp = 0;
//	mSvrPort = 0;
//	FSET_DISCNN();
//	procClosed();
}

void BaseConnection::OnIdle() {
}

void BaseConnection::startIdleTimer() {
	mCnnTimer.set(5000, 0, [this](){
		ald("idle timer expired...");
		close();
		FSET_DISCNN();
		OnDisconnected();
	});
}

void BaseConnection::forceCloseChannel(uint32_t rx, uint32_t tx) {
	ald("force close channel, rx=%d, tx=%d", rx, tx);
	for(auto itr=mRxChList.begin(); itr!=mRxChList.end(); itr++) {
		if(itr->handle == rx) {
			mDummyChannels.splice(mDummyChannels.end(), mRxChList, itr);
			break;
		}
	}

	for(auto itr=mTxChList.begin(); itr!=mTxChList.end(); itr++) {
		if(itr->handle == tx) {
			mDummyChannels.splice(mDummyChannels.end(), mTxChList, itr);
			break;
		}
	}
	mSocket.close();
	mCnnTimer.kill();
	mLocalEvent.postEvent(0, 0, 0);
}

} /* namespace cahttp */
