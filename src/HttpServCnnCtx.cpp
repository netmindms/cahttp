/*
 * HttpServCnnMan.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: root
 */

#define LOG_LEVEL LOG_INFO
#include "flog.h"
#include "HttpServCnnCtx.h"
#include "CaHttpServer.h"
namespace cahttp {
HttpServCnnCtx::HttpServCnnCtx() {
	mHandleSeed = 0;
	mpSvr = nullptr;
}

HttpServCnnCtx::~HttpServCnnCtx() {
}

void HttpServCnnCtx::init(CaHttpServer& svr, int maxn) {
	mpSvr = &svr;
}

void HttpServCnnCtx::newCnn(int fd) {
	if (++mHandleSeed == 0)
		mHandleSeed++;
	auto mres = mCnnList.emplace(mHandleSeed, mHandleSeed); // TODO: ServCNN() remove
//	auto mres = mCnnList.emplace(mHandleSeed, ServCnn()); // TODO: ServCNN() remove
	mres.first->second.open(fd, mHandleSeed, this);
	mpSvr->notifyNewConnection(mres.first->second);
}

void HttpServCnnCtx::close() {
	for (auto &kv : mCnnList) {
		kv.second.close();
	}
	mCnnList.clear();
}

void HttpServCnnCtx::freeCnn(uint32_t handle) {
	auto itr = mCnnList.find(handle);
	if (itr != mCnnList.end()) {
		mCnnList.erase(itr);
	}
	ali("handle=%x connection erased from map, count=%d", handle, mCnnList.size());
}


const UrlMap* HttpServCnnCtx::getUrlMap(http_method method) {
	return mpSvr->getUrlMap(method);
}

void HttpServCnnCtx::disconnectedCnn(ServCnn* pcnn) {
	mpSvr->notifyCloseConnection(*pcnn);
}

CaHttpUrlCtrl* HttpServCnnCtx::findAndAlloc(http_method method, const string& urlstr) {
	auto *urlmap = getUrlMap(method);
	if(urlmap) {
		auto itr = urlmap->find(urlstr);
		if (itr != urlmap->end()) {
			ald("static url control found,");
			auto* pctrl = itr->second();
			auto h = mHandleFac.newHandle();
			pctrl->setHandle(h);
			return pctrl;
		} else {
			ali("*** static url not found");
		}
	}
	smatch mresult;
	auto alloc = mpSvr->matchRegExUrl(mresult, urlstr);
	if (alloc) {
		auto &ctrl = *(alloc());
		auto h = mHandleFac.newHandle();
		ctrl.setHandle(h);
		ctrl.setUrlMatchResult(mresult);
		return &ctrl;
	}
	return nullptr;
}


}
