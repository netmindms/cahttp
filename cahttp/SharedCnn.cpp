/*
 * SharedCnn.cpp
 *
 *  Created on: Mar 20, 2016
 *      Author: netmind
 */

#include "SharedCnn.h"
#include "BaseConnection.h"
#include "HttpCnnMan.h"

namespace cahttp {

SharedCnn::SharedCnn() {
	mpCnnMan = nullptr;
	mHandle = 0;
	mRxCh =0;
	mTxCh = 0;
	mpPipeCnn = nullptr;
}

SharedCnn::~SharedCnn() {
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
	mLis = lis;
	auto res = mpCnnMan->connect(ip, port);
	mpPipeCnn = res.first;
	mpPipeCnn->openRxCh([this](BaseConnection::CH_E evt) {
		if(evt == CH_E::CH_MSG) {
			mRecvMsg.reset( mpPipeCnn->fetchMsg());
			mLis(BaseCnn::CH_MSG);
		} else if(evt == CH_E::CH_DATA) {
//			mRecvData = m
		}
		return 0;
	});
	return res.second;
}

void SharedCnn::close() {
	if(mRxCh || mTxCh) {
		mpPipeCnn->forceCloseChannel(mRxCh, mTxCh);
	}
}

} /* namespace cahttp */
