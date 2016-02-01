/*
 * BaseConnection.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include <memory>
#include "BaseMsg.h"
#include "BaseConnection.h"
#include "flog.h"

#include "util.h"

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
	// TODO Auto-generated destructor stub
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
			FSET_DISCNN();
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
		ali("*** not connected");
		return 1;
	}
	if(mSocket.isWritable()) {
		return mSocket.sendPacket(buf, len);
	} else {
		ali("*** not writable");
		return 1;
	}
}

void BaseConnection::endSend(uint32_t handle) {
}

void BaseConnection::close() {
	mSocket.close();
}

//
//void BaseConnection::setCallback(CnnIf* pif) {
//	mNotiIf = pif;
//}

//void BaseConnection::OnRecvMsg(CaHttpMsg& msg) {
//}
//
//void BaseConnection::OnRecvData(string& data) {
//}

int BaseConnection::procRead() {
	auto rcnt = mSocket.recvPacket(mBuf, mBufSize);
	if(rcnt>0) {
		auto ccnt = mMsgFrame.feedPacket(mBuf, mBufSize);
		if(ccnt) {
			assert(ccnt == rcnt);
			for(;;) {
				auto fetch_status = mMsgFrame.status();
				if(fetch_status == mMsgFrame.FS_HDR) {
					unique_ptr<BaseMsg> upmsg( new BaseMsg );
					auto fetch_result = mMsgFrame.fetchMsg(*upmsg);
					mNotiIf->OnMsg(move(upmsg));
				} else if(fetch_status == mMsgFrame.FS_DATA) {
					string data;
					auto fetch_result = mMsgFrame.fetchData(data);
					mNotiIf->OnData(move(data));
				} else if(fetch_status == mMsgFrame.FS_NONE) {
					break;
				}
			}
		} else {
			alw("*** http msg parser error");
			return -2;
		}
	} else {
		alw("*** no read data");
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
