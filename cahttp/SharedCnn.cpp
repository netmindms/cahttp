/*
 * SharedCnn.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: netmind
 */

#include "SharedCnn.h"
#include "BaseConnection.h"
#include "HttpCnnMan.h"

using namespace std;

namespace cahttp {

SharedCnn::SharedCnn() {
	mpCnnMan = nullptr;
	mHandle = 0;
	mRxCh =0;
	mTxCh = 0;
	mpPipeCnn = nullptr;
}

SharedCnn::~SharedCnn() {
	close();
}

cahttp::SR SharedCnn::send(const char* buf, size_t len) {
	return mpPipeCnn->send(mTxCh, buf, len);
}

void SharedCnn::sendEnd() {
	mpPipeCnn->endTxCh(mTxCh); mTxCh = 0;
}

void SharedCnn::recvEnd() {
	mpPipeCnn->endRxCh(mRxCh); mRxCh = 0;
}

void SharedCnn::setHttpCnnMan(HttpCnnMan& cnnman) {
}

int SharedCnn::connect(uint32_t ip, int port, int timeout, std::function<void(CH_E)> lis) {
	return 0;
}

void SharedCnn::close() {
	if(mRxCh || mTxCh) {
		mpPipeCnn->forceCloseChannel(mRxCh, mTxCh);
	}
	mpPipeCnn.reset();
}


void SharedCnn::openSharedCnn(shared_ptr<BaseConnection> spcnn) {
	mpPipeCnn = spcnn;
	mRxCh = spcnn->openRxCh([this](BaseConnection::CH_E evt) {
		if(evt == BaseConnection::CH_MSG) {
			auto pmsg = mpPipeCnn->fetchMsg();
			mRecvMsg.reset(pmsg);
			mLis(BaseCnn::CH_MSG);
		} else if(evt == BaseConnection::CH_DATA) {
			mRecvData.clear();
			mRecvData = mpPipeCnn->fetchData();
			mLis(BaseCnn::CH_DATA);
		} else if(evt == BaseConnection::CH_CLOSED) {
			mRxCh = 0;
			procClosed();
		}
		return 0;
	});
	mTxCh = spcnn->openTxCh([this](BaseConnection::CH_E evt) {
		if(evt == BaseConnection::CH_WRITABLE) {
			procWritable();
		} else if(evt == BaseConnection::CH_CLOSED) {
			mTxCh = 0;
			procClosed();
		}
		return 0;
	});
}

} /* namespace cahttp */
