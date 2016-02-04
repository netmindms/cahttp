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
#include "flog.h"

using namespace edft;
using namespace std;

#define FP_CNN 0

#define FSET_CNN() BIT_SET(mStatusFlag, FP_CNN)
#define FSET_DISCNN() BIT_RESET(mStatusFlag, FP_CNN)
#define FGET_CNN() BIT_TEST(mStatusFlag, FP_CNN)

namespace cahttp {

BaseConnection::BaseConnection() {
	mBuf = nullptr;
	mBufSize = 2048;
	mNotiIf = nullptr;
	mStatusFlag = 0;
}

BaseConnection::~BaseConnection() {
	alv("dest basecnn, ptr=%x", (long)this);
	close();
	if(mBuf) {
		delete[] mBuf;
	}
}


int BaseConnection::connect(uint32_t ip, int port) {
	if(!mBuf) {
		mBuf = new char[mBufSize];
		assert(mBuf);
	}

	mMsgFrame.init(false);

	mSocket.setOnListener([this](EdSmartSocket& sck, int event) {
		if(event == NETEV_CONNECTED) {
			FSET_CNN();
			mNotiIf->OnWritable();
		} else if(event == NETEV_DISCONNECTED) {
			ali("*** disconnected...");
			FSET_DISCNN();
			mNotiIf->OnCnn(0);
		} else if(event == NETEV_READABLE) {
			procRead();
		} else if(event == NETEV_WRITABLE) {
			mNotiIf->OnWritable();
		}
	});
	return mSocket.connect(ip, port);
}

int BaseConnection::send(const char* buf, size_t len) {
	if(FGET_CNN()==0) {
		ald("*** not yet connected");
		return 1;
	}
	if(mSocket.isWritable()) {
		auto wret = mSocket.sendPacket(buf, len);
		if(wret==SEND_OK) return 0;
		else if(wret == SEND_PENDING) return -1;
		else return 1;
	} else {
		ald("*** not writable");
		return 1;
	}
}

void BaseConnection::endSend(uint32_t handle) {
}

void BaseConnection::close() {
	ali("close socket, fd=%d, cnnptr=%x", mSocket.getFd(), (long)this);
	mSocket.close();
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
		ald("packet data:\n|%s|", string(mBuf, rcnt));
		if(ccnt) {
			assert(ccnt == rcnt);
			int bexit=0;
			for(;!bexit;) {
				auto fetch_status = mMsgFrame.status();
				if(fetch_status == mMsgFrame.FS_HDR) {
					unique_ptr<BaseMsg> upmsg( new BaseMsg );
					auto fetch_result = mMsgFrame.fetchMsg(*upmsg);
					bexit = mNotiIf->OnMsg(move(upmsg));
				} else if(fetch_status == mMsgFrame.FS_DATA) {
					string data;
					auto fetch_result = mMsgFrame.fetchData(data);
					bexit = mNotiIf->OnData(move(data));
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
		//ali("*** no read data");
	}
	return 0;
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

} /* namespace cahttp */
