/*
 * ReqMan.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#include "ReqMan.h"

namespace cahttp {

ReqMan::ReqMan() {
	// TODO Auto-generated constructor stub

}

ReqMan::~ReqMan() {
	// TODO Auto-generated destructor stub
}

HttpReq* ReqMan::getReq(uint32_t ip, int port) {
	SharedConnection* cnn=nullptr;
	for(auto &c: mCnns) {
		if(ip == c.second.ip() && port == c.second.port()) {
			cnn = &c.second;
		}
	}
	if(cnn == nullptr) {
		if(++mHandleSeed==0) mHandleSeed++;
		cnn = &(mCnns[mHandleSeed]);
	}

	if(++mHandleSeed==0) mHandleSeed++;
	auto &req = mReqs[mHandleSeed];
	req.setConnection(cnn);
	return &(req);
}

} /* namespace cahttp */
