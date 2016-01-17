/*
 * CaHttpSvrReq.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: netmind
 */

#include "CaHttpSvrReq.h"

#include "HttpStringReadStream.h"
namespace cahttp {
CaHttpSvrReq::CaHttpSvrReq() {
	// TODO Auto-generated constructor stub

}

CaHttpSvrReq::~CaHttpSvrReq() {
	// TODO Auto-generated destructor stub
}

http_method CaHttpSvrReq::getMethod() {
	return mReqMsg.getMethod();
}

void CaHttpSvrReq::setReqMsg(CaHttpMsg&& msg) {
	mReqMsg = move(msg);
}


HttpBaseWriteStream* CaHttpSvrReq::getReqWriteStream() {
	return mReqWriteStream.get();
}

void CaHttpSvrReq::buildBaseRespMsg(CaHttpMsg& msg, int status) {
	msg.setResponse(status);

}

const CaHttpMsg& CaHttpSvrReq::getReqMsg() const {
	return mReqMsg;
}
}
