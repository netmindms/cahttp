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
	ald("new incoming cnn, fd=%d", fd);
	if(++mHandleSeed==0) ++mHandleSeed;
	auto &scnn = mCnns[mHandleSeed];
	ald("      handle=%d, cnn_count=%d", mHandleSeed, mCnns.size());
	return scnn.init(mHandleSeed, fd, *this);
}

void ReHttpSvrCtx::init(ReHttpServer& svr) {
	mpSvr = &svr;
}

void ReHttpSvrCtx::dummyCnn(uint32_t handle) {
	ald("goto dummy cnn, handle=%d", handle);
	mCnnDummy.push_back(handle);
}

void ReHttpSvrCtx::clearCnnDummy() {
	ali("clear cnn dummy, cnt=%d", mCnnDummy.size());
	for(auto &h: mCnnDummy) {
		mCnns.erase(h);
	}
	mCnnDummy.clear();
}


void ReHttpSvrCtx::close() {
	mCnns.clear();
}

} /* namespace cahttp */
