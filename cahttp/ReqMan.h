/*
 * ReqMan.h
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REQMAN_H_
#define CAHTTP_REQMAN_H_
#include <unordered_map>

#include "HttpManReq.h"
#include "SharedConnection.h"

namespace cahttp {

class ReqMan {
public:
	ReqMan();
	virtual ~ReqMan();
	HttpReq* getReq(uint32_t ip, int port);
private:
	std::unordered_map<uint32_t, SharedConnection> mCnns;
	std::unordered_map<uint32_t, HttpManReq> mReqs;
	uint32_t mHandleSeed;
};

} /* namespace cahttp */

#endif /* CAHTTP_REQMAN_H_ */
