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
#if 0
	mNotiIf = nullptr;
	mRecvIf = nullptr;
#endif
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
	if(!mBuf) {
		mBuf = new char[mBufSize];
		assert(mBuf);
	}

#if 1
	init_sock(false, 0);
#else
	mMsgFrame.init(false);
	mSocket.setOnListener([this](EdSmartSocket& sck, int event) {
		if(event == NETEV_CONNECTED) {
			FSET_CNN();
//			mNotiIf->OnWritable();
			procWritable();
		} else if(event == NETEV_DISCONNECTED) {
			ald("*** disconnected...");
			FSET_DISCNN();
//			mNotiIf->OnCnn(0);
			procClosed();
		} else if(event == NETEV_READABLE) {
			procRead();
		} else if(event == NETEV_WRITABLE) {
//			mNotiIf->OnWritable();
			procWritable();
		}
	});
#endif

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

#if 0
void BaseConnection::endSend(uint32_t handle) {

}

void BaseConnection::changeSend(CnnIf* pif) {
	mNotiIf = pif;
}



uint32_t BaseConnection::startSend(CnnIf* pif) {
	if(!mNotiIf) {
		mNotiIf = pif;
		return 1; // TODO:
	} else {
		ale("### Error: connection callback if exists");
		assert(0);
		return 0;
	}
}
#endif

void BaseConnection::close() {
	if(mSocket.getFd()>=0) {
		ald("close socket, fd=%d, cnnptr=%x", mSocket.getFd(), (long)this);
		mSocket.close();
	}

	mCnnTimer.kill();

}

void BaseConnection::reserveWrite() {
	mSocket.reserveWrite();
}


int BaseConnection::procRead() {
	assert(mSocket.getFd()>0);
//	assert(mNotiIf);
	alv("proc read, fd=%d, cnnptr=%x", mSocket.getFd(), (long)this);
	auto rcnt = mSocket.recvPacket(mBuf, mBufSize);
	if(rcnt>0) {
		auto ccnt = mMsgFrame.feedPacket(mBuf, rcnt);
		ald("consumed cnt in parser, cnt=%d", ccnt);
		alv("packet data:\n|%s|", string(mBuf, rcnt));
		if(ccnt) {
			assert(ccnt == rcnt);
			int bexit=0;
			for(;!bexit;) {
				auto fetch_status = mMsgFrame.status();
				if(fetch_status == mMsgFrame.FS_HDR) {
					mRecvMsg.reset( new BaseMsg );
					auto fetch_result = mMsgFrame.fetchMsg( (*mRecvMsg) );
//					bexit = mRecvIf->OnMsg(move(mRecvMsg));
					if(mRxChList.size()) {
						bexit = mRxChList.front().lis(CH_MSG);
					} else {
						assert(mDefRxLis);
						bexit = mDefRxLis(CH_MSG);
					}
				} else if(fetch_status == mMsgFrame.FS_DATA) {
					mRecvData.clear();
					auto fetch_result = mMsgFrame.fetchData(mRecvData);
//					bexit = mRecvIf->OnData(move(data));
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
#if 0
	mSocket.setOnListener([this](EdSmartSocket& sck, int event) {
		if(event == NETEV_CONNECTED) {
			FSET_CNN();
			if(mNotiIf) mNotiIf->OnWritable();
		} else if(event == NETEV_DISCONNECTED) {
			ald("*** disconnected...");
			FSET_DISCNN();
			if(mNotiIf) mNotiIf->OnCnn(0);
		} else if(event == NETEV_READABLE) {
			procRead();
		} else if(event == NETEV_WRITABLE) {
			if(mNotiIf) mNotiIf->OnWritable();
		}
	});
#endif
	mSocket.setOnListener([this](int event) {
			if(event == NETEV_CONNECTED) {
				ald("sock connected");
				mCnnTimer.kill();
				FSET_CNN();
	//			mNotiIf->OnWritable();
				procWritable();
			} else if(event == NETEV_DISCONNECTED) {
				ald("*** sock disconnected...");
				mCnnTimer.kill();
				mSocket.close();
				FSET_DISCNN();
	//			mNotiIf->OnCnn(0);
				procClosed();
			} else if(event == NETEV_READABLE) {
				alv("sock readble");
				procRead();
			} else if(event == NETEV_WRITABLE) {
				alv("sock writable");
	//			mNotiIf->OnWritable();
				procWritable();
			}
	});
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
	if(++mHandleSeed==0) mHandleSeed++;
	_chlis *c;
	mRxChList.emplace_back();
	c = &(mRxChList.back());
	c->handle = mHandleSeed;
	c->lis = lis;
	return mHandleSeed;
}

void BaseConnection::endTxCh(uint32_t h) {
	assert(mTxChList.size()>0 && mTxChList.front().handle==h);
	for(auto itr=mTxChList.begin();itr != mTxChList.end(); itr++) {
		if(itr->handle == h) {
			mTxChList.erase(itr);
			break;
		}
	}
	if(mTxChList.size()) {
//		reserveWrite();
		procWritable();
	}
}

void BaseConnection::endRxCh(uint32_t h) {
	ald("end rx ch=%d", h);
	for(auto itr=mRxChList.begin();itr != mRxChList.end(); itr++) {
		if(itr->handle == h) {
			mRxChList.erase(itr);
			break;
		}
	}
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

} /* namespace cahttp */
