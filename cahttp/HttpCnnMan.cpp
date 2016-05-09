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
#include "BaseConnection.h"

using namespace std;

namespace cahttp {

HttpCnnMan::HttpCnnMan() {
	mHandleSeed = 0;
	mCfg.val = 0;
}

HttpCnnMan::~HttpCnnMan() {
	// TODO Auto-generated destructor stub
}

pair<shared_ptr<SharedCnn>, int> HttpCnnMan::connect(uint32_t ip, uint16_t port) {
#if 1
	return connect_pipeline(ip, port);
#else
	if(mCfg.pipelining==0) {
		return connect_base(ip, port);
	} else {
		return connect_pipeline(ip, port);
	}
#endif
}


pair<shared_ptr<SharedCnn>, int> HttpCnnMan::connect_pipeline(uint32_t ip, uint16_t port) {
	shared_ptr<BaseConnection> spcnn;

	// search existing connection
	for(auto &c: mPipeCnnPool) {
		auto addr = c->getRmtAddr();
		if(addr.first == ip && addr.second == port) {
			if(!mCfg.pipelining) {
				spcnn = c;
				break;
			} else {
				if(c->isIdle()) {
					spcnn = c;
					break;
				}
			}
		}
	}

	if(spcnn==nullptr) {
		// not found
		spcnn = make_shared<BaseConnection>();
		if(++mHandleSeed==0) mHandleSeed++;
		auto handle = mHandleSeed;
		spcnn->setHandle(handle);
		spcnn->setDefRxListener([this, handle](BaseConnection::CH_E evt) {
			if(evt == BaseConnection::CH_CLOSED) {
				ald("disconnected pipeline cnn, handle=%ld", handle);
				for(auto itr=mPipeCnnPool.begin(); itr!=mPipeCnnPool.end(); itr++) {
					if((*itr)->getHandle()==handle) {
						ald("cnn ref_count=%ld", (*itr).use_count());
						(*itr)->close();
						mPipeCnnPool.erase(itr);
						ald("delete connection, handle=%ld, pool_size=%d", handle, mPipeCnnPool.size());
						break;
					}
				}
			}
			return 0;
		});
		mPipeCnnPool.push_back(spcnn);
		ali("new shared connection for %s:%d, cnt=%ld", cahttpu::Ip2Str(ip), port, mPipeCnnPool.size());
	}

	auto cret = spcnn->connect(ip, port);
	shared_ptr<SharedCnn> upcnn(new SharedCnn);
	upcnn->openSharedCnn(spcnn);
	return {move(upcnn), cret};
}

#if 0
std::pair<std::shared_ptr<SimpleCnn>, int> HttpCnnMan::connect_base(uint32_t ip, uint16_t port) {
	shared_ptr<SimpleCnn> res;
	for(auto itr=mBaseCnnPool.begin(); itr != mBaseCnnPool.end(); itr++) {
		auto rmtaddr = (*itr)->getRmtAddr();
		if(rmtaddr.first == ip && rmtaddr.second == port) {
			res = *itr;
			break;
		}
	}
	if(res == nullptr) {
		res.reset(new SimpleCnn);
		mBaseCnnPool.push_back(res);
	}
	auto ret = res->connect(ip, port, 30000, nullptr);
	return {move(res), ret} ;
}
#endif

void HttpCnnMan::close() {
	for(auto &c: mPipeCnnPool) {
		c->close();
	}
	mPipeCnnPool.clear();
}

pair<size_t, size_t> HttpCnnMan::getChannelCount(size_t idx) {
	if(idx < mPipeCnnPool.size()) {
		auto itr = mPipeCnnPool.begin();
		for(auto i=0;i<idx;i++) itr++;
		return { (*itr)->getRxChCount(), (*itr)->getTxChCount() };
	} else {
		assert(0);
		return {0, 0};
	}
}


} /* namespace cahttp */
