/*
 * CaHttpReqMan.cpp
 *
 *  Created on: Dec 4, 2015
 *      Author: netmind
 */

#include "CaHttpReqMan.h"
namespace cahttp {
CaHttpReqMan::CaHttpReqMan() {
}

CaHttpReqMan::~CaHttpReqMan() {
}

CaHttpReq* CaHttpReqMan::newRequest() {
	mReqList.emplace_back(new CaHttpReq);
	auto req = mReqList.back();
	req->setReqMan(this);
	return req;
}

void CaHttpReqMan::close() {
	for(auto preq: mReqList) {
		preq->close();
	}
	mCnnList.clear();
	clearDummyReq();
}

void CaHttpReqMan::gotoDummyReq(CaHttpReq* preq) {
	mDummyReqList.emplace_back(preq);
}

void CaHttpReqMan::clearDummyReq() {
	for(;mDummyReqList.size()>0;) {
		auto preq = mDummyReqList.front();
		mDummyReqList.pop_front();
		delete preq;
	}
}

HttpCnn* CaHttpReqMan::getConnection(uint32_t ipaddr, int port) {
	cnninfo_t *pi=nullptr;
	for(auto &ci : mCnnList) {
		if(ci.ip == ipaddr && ci.port == port) {
			pi = &ci;
			break;
		}
	}
	if(!pi) {
		mCnnList.emplace_back();
		pi = &(mCnnList.back());
		pi->ip = ipaddr;
		pi->port = port;
		pi->cnn.setHostIpAddr(ipaddr, port);
	}
	return &(pi->cnn);
}

void CaHttpReqMan::freeRequest(CaHttpReq* preq) {
	gotoDummyReq(preq);
}

}
