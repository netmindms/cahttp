/*
 * BaseCnn.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE

#include <memory>
#include "BaseMsg.h"
#include "SimpleCnn.h"
#include "ext/nmdutil/etcutil.h"
#include "ext/nmdutil/netutil.h"
#include "flog.h"

using namespace edft;
using namespace std;

namespace cahttp {

SimpleCnn::SimpleCnn() {
	mStatus.val = 0;
	mBuf = nullptr;
	mBufSize = 2048;
	mStatusFlag = 0;
	mHandleSeed=0;
	mSvrIp = 0;
	mSvrPort = 0;
}

SimpleCnn::~SimpleCnn() {
	alv("dest basecnn, ptr=%x", (long)this);
	close();
	if(mBuf) {
		delete[] mBuf;
	}
}


int SimpleCnn::connect(uint32_t ip, int port, int timeout,std::function<void(CH_E)> lis) {
	mLis = lis;
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
		mStatus.cnn_status = 0;
		procClosed();
	});
	mCnnTimer.set(timeout);
	return mSocket.connect(ip, port);

}

SR SimpleCnn::send(const char* buf, size_t len) {
	if(mStatus.cnn_status==0) {
		ald("*** not yet connected");
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

void SimpleCnn::close() {
	if(mSocket.getFd()>=0) {
		ald("close socket, fd=%d, cnnptr=%x", mSocket.getFd(), (long)this);
		mSocket.close();
	}

	mCnnTimer.kill();
	mLocalEvent.close();
	mSvrIp = 0;
	mSvrPort = 0;
}

void SimpleCnn::reserveWrite() {
	mSocket.reserveWrite();
}


int SimpleCnn::procRead() {
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
					mLis(CH_MSG);
				} else if(fetch_status == mMsgFrame.FS_DATA) {
					mRecvData.clear();
					mMsgFrame.fetchData(mRecvData);
					mLis(CH_DATA);
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



int SimpleCnn::openServer(int fd) {
	mStatus.server=1;
	init_sock(true, fd);
	return 0;
}


void SimpleCnn::init_sock(bool svr, int fd) {
	if(!mBuf) {
		mBuf = new char[mBufSize];
		assert(mBuf);
	}
	mMsgFrame.init(svr);
	if(svr) {
		mSocket.openChild(fd);
		mStatus.cnn_status=1;
	}

	mSocket.setOnListener([this](int event) {
			if(event == NETEV_CONNECTED) {
				ald("sock connected");
				mCnnTimer.kill();
				mStatus.cnn_status=1;
				procWritable();
			} else if(event == NETEV_DISCONNECTED) {
				ald("*** sock disconnected...");
				mCnnTimer.kill();
				mSocket.close();
				mStatus.cnn_status=0;
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
//				mDummyChannels.clear();
				procClosed();
				OnIdle();
			}
		});
	}
}




int SimpleCnn::procWritable() {
	mLis(CH_WRITABLE);
	return 0;
}

int SimpleCnn::procClosed() {
	mLis(CH_CLOSED);
	return 0;
}

void SimpleCnn::OnDisconnected() {
//	mCnnTimer.kill();
//	mSocket.close();
//	mSvrIp = 0;
//	mSvrPort = 0;
//	FSET_DISCNN();
//	procClosed();
}

void SimpleCnn::OnIdle() {
}

void SimpleCnn::sendEnd() {
}

void SimpleCnn::recvEnd() {
}

void SimpleCnn::startIdleTimer() {
	mCnnTimer.set(5000, 0, [this](){
		ald("idle timer expired...");
		close();
		mStatus.cnn_status=0;
		OnDisconnected();
	});
}

void SimpleCnn::forceCloseChannel(uint32_t rx, uint32_t tx) {
	ald("force close channel, rx=%d, tx=%d", rx, tx);

	mSocket.close();
	mCnnTimer.kill();
	mSvrIp = 0;
	mSvrPort = 0;
//	mLocalEvent.postEvent(0, 0, 0);
	procClosed();
}

} /* namespace cahttp */
