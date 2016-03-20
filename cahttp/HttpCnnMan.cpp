/*
 * HttpCnnMan.cpp
 *
 *  Created on: Mar 12, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG

#include "HttpCnnMan.h"
#include "flog.h"
#include "ext/nmdutil/netutil.h"

namespace cahttp {

HttpCnnMan::HttpCnnMan() {
	mHandleSeed = 0;
	mCfg.val = 0;
}

HttpCnnMan::~HttpCnnMan() {
	// TODO Auto-generated destructor stub
}

std::pair<std::shared_ptr<BaseCnn>, int> HttpCnnMan::connect(uint32_t ip, uint16_t port) {
	for(auto &c: mCnnPool) {
		auto addr = c->getRmtAddr();
		if(addr.first == ip && addr.second == port) {
			if(!mCfg.pipelining) {
				if(c->isIdle()==false) {
					auto r = c->connect(ip, port);
					return {c, r};
					break;
				}
			} else {
				auto r = c->connect(ip, port);
				return {c, r};
				break;
			}
		}
	}

	auto pcnn = std::make_shared<SharedConnection>();
	if(++mHandleSeed==0) mHandleSeed++;
	pcnn->setHandle(mHandleSeed);
	pcnn->setRelLis([this](uint32_t handle) {
		for(auto itr=mCnnPool.begin(); itr!=mCnnPool.end(); itr++) {
			if((*itr)->getHandle()==handle) {
				ald("cnn ref_count=%ld", (*itr).use_count());
				(*itr)->terminate();
				ald("delete connection, handle=%ld, pool_size=%d", handle, mCnnPool.size());
				mCnnPool.erase(itr);
				break;
			}
		}
	});
	mCnnPool.push_back(pcnn);
	ali("new shared connection for %s:%d, cnt=%ld", cahttpu::Ip2Str(ip), port, mCnnPool.size());
	auto r = pcnn->connect(ip, port, 30000);
	return {pcnn, r};
}

void HttpCnnMan::close() {
	for(auto &c: mCnnPool) {
		c->terminate();
	}
	mCnnPool.clear();
}

} /* namespace cahttp */
