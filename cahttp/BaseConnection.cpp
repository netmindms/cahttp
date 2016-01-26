/*
 * BaseConnection.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include "BaseConnection.h"
#include "flog.h"

using namespace edft;
using namespace std;

namespace cahttp {

BaseConnection::BaseConnection() {
	mBuf = nullptr;
	mBufSize = 2048;
}

BaseConnection::~BaseConnection() {
	// TODO Auto-generated destructor stub
}


int BaseConnection::connect(const std::string& ip, int port) {
	if(!mBuf) {
		mBuf = new char[mBufSize];
		assert(mBuf);
	}

	mMsgFrame.init(false);

	mSocket.setOnListener([this](EdSmartSocket& sck, int event) {
		if(event == NETEV_CONNECTED) {

		} else if(event == NETEV_DISCONNECTED) {

		} else if(event == NETEV_READABLE) {
			procRead();
		} else if(event == NETEV_WRITABLE) {

		}
	});
	return 0;
}

int BaseConnection::procRead() {
	auto rcnt = mSocket.recvPacket(mBuf, mBufSize);
	if(rcnt>0) {
		auto ccnt = mMsgFrame.feedPacket(mBuf, mBufSize);
		if(ccnt) {
			assert(ccnt == rcnt);
			for(;;) {
				auto fetch_status = mMsgFrame.status();
				if(fetch_status == mMsgFrame.FS_HDR) {
//					auto fetch_result = mMsgFrame.fetchMsg()
				} else if(fetch_status == mMsgFrame.FS_DATA) {

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

} /* namespace cahttp */
