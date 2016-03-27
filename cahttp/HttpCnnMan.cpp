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

pair<unique_ptr<SharedCnn>, int> HttpCnnMan::connect(uint32_t ip, uint16_t port) {
	shared_ptr<BaseConnection> spcnn;
	for(auto &c: mCnnPool) {
		auto addr = c->getRmtAddr();
		if(addr.first == ip && addr.second == port) {
			spcnn = c;
			break;
		}
	}


	if(spcnn==nullptr) {
		// not found
		spcnn = make_shared<BaseConnection>();
		if(++mHandleSeed==0) mHandleSeed++;
		auto handle = mHandleSeed;
		spcnn->setHandle(handle);
		spcnn->setDefRxListener([this, handle](BaseConnection::CH_E evt) {
			for(auto itr=mCnnPool.begin(); itr!=mCnnPool.end(); itr++) {
				if((*itr)->getHandle()==handle) {
					ald("cnn ref_count=%ld", (*itr).use_count());
					(*itr)->close();
					ald("delete connection, handle=%ld, pool_size=%d", handle, mCnnPool.size());
					mCnnPool.erase(itr);
					break;
				}
			}
			return 0;
		});
		mCnnPool.push_back(spcnn);
		ali("new shared connection for %s:%d, cnt=%ld", cahttpu::Ip2Str(ip), port, mCnnPool.size());
	}

	auto cret = spcnn->connect(ip, port);
	unique_ptr<SharedCnn> upcnn(new SharedCnn);
	upcnn->openSharedCnn(spcnn);
	return {move(upcnn), cret};
}

void HttpCnnMan::close() {
	for(auto &c: mCnnPool) {
		c->close();
	}
	mCnnPool.clear();
}

} /* namespace cahttp */
