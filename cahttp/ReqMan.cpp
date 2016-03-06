/*
 * ReqMan.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_VERBOSE


#include "ReqMan.h"
#include "ext/nmdutil/netutil.h"
#include "flog.h"

namespace cahttp {

ReqMan::ReqMan() {
	mHandleSeed = 0;
}

ReqMan::~ReqMan() {
	// TODO Auto-generated destructor stub
}

HttpReq* ReqMan::getReq(uint32_t ip, int port) {
	clearDummyReq();
	SharedConnection* cnn=nullptr;
	for(auto &c: mCnns) {
		if(ip == c.ip() && port == c.port()) {
			ald("connection already exist");
			cnn = &c;
		}
	}
	if(cnn == nullptr) {
		ald("cnn not found, ip=%s, port=%d", cahttpu::Ip2Str(ip), port);
		if(++mHandleSeed==0) mHandleSeed++;
		mCnns.emplace_back();
		cnn = &(mCnns.back());
		cnn->setHandle(mHandleSeed);
		cnn->setRelLis([this](uint32_t handle) {
			for(auto itr=mCnns.begin(); itr!=mCnns.end();itr++) {
				if(handle == itr->getHandle()) {
					mCnnDummyPool.splice(mCnnDummyPool.end(), mCnns, itr);
					break;
				}
			}
			ald("dummy cnn pool size=%d", mCnnDummyPool.size());
		});
		ald("add new connection, handle=%ld, cnt=%d", mHandleSeed, mCnns.size());
	}

	if(++mHandleSeed==0) mHandleSeed++;
	auto &req = mReqs[mHandleSeed];
	req.init(*this, mHandleSeed);
	req.setConnection(cnn);
	ald("new request, handle=%ld, cnt=%d", mHandleSeed, mReqs.size());
	return &(req);
}


void ReqMan::dummyReq(uint32_t handle) {
	mReqDummy.push_back(handle);
	ald("add req to dummy, handle=%ld, cnt=%d", handle, mReqDummy.size());
}


void ReqMan::clearDummyReq() {
	for(auto &r: mReqDummy) {
		mReqs.erase(r);
	}
}


void ReqMan::close() {
	clearDummyReq();
	mReqs.clear();

	for(auto &cnn: mCnns) {
		cnn.close();
	}
	mCnns.clear();

}

} /* namespace cahttp */
