/*
 * HttpManReq.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#include "ReqMan.h"
#include "HttpManReq.h"

namespace cahttp {

HttpManReq::HttpManReq() {
	mHandle = 0;
}

HttpManReq::~HttpManReq() {
	// TODO Auto-generated destructor stub
}

void HttpManReq::close() {
	HttpReq::close();
	if(mHandle) {
		assert(mpReqMan);
		mpReqMan->dummyReq(mHandle);
		mHandle=0;
		mpReqMan = nullptr;
	}
}

} /* namespace cahttp */
