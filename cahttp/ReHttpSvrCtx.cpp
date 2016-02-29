/*
 * ReHttpSvrCtx.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG

#include "ReHttpSvrCtx.h"

#include "CaHttpUrlParser.h"
#include "flog.h"

namespace cahttp {

ReHttpSvrCtx::ReHttpSvrCtx() {
	mHandleSeed = 0;
	mpSvr = nullptr;
}

ReHttpSvrCtx::~ReHttpSvrCtx() {
	// TODO Auto-generated destructor stub
}


int ReHttpSvrCtx::newCnn(int fd) {
	clearCnnDummy();
	if(++mHandleSeed==0) ++mHandleSeed;
	auto &scnn = mCnns[mHandleSeed];
	ald("new incoming connection, fd=%d, handle=%d, cnn_count=%d", fd, mHandleSeed, mCnns.size());
	return scnn.init(mHandleSeed, fd, *this);
}

void ReHttpSvrCtx::init(ReHttpServer& svr) {
	mpSvr = &svr;
}

void ReHttpSvrCtx::dummyCnn(uint32_t handle) {
	mCnnDummy.push_back(handle);
	ald("goto dummy cnn, h=%d, cnt=%d", handle, mCnnDummy.size());
}

void ReHttpSvrCtx::clearCnnDummy() {
	ali("clear cnn dummy, cnt=%d", mCnnDummy.size());
	for(auto &h: mCnnDummy) {
		mCnns.erase(h);
	}
	mCnnDummy.clear();
}


void ReHttpSvrCtx::close() {
	for(auto &c: mCnns) {
		c.second.close();
	}
	mCnns.clear();

}

} /* namespace cahttp */
