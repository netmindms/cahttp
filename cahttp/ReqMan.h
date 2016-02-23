/*
 * ReqMan.h
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REQMAN_H_
#define CAHTTP_REQMAN_H_
#include <list>
#include <unordered_map>

#include "HttpManReq.h"
#include "SharedConnection.h"

namespace cahttp {

class ReqMan {
	friend class HttpManReq;
public:
	ReqMan();
	virtual ~ReqMan();
	HttpReq* getReq(uint32_t ip, int port);
	void close();
private:
	std::unordered_map<uint32_t, SharedConnection> mCnns;
	std::unordered_map<uint32_t, HttpManReq> mReqs;
	std::list<uint32_t> mReqDummy;
	uint32_t mHandleSeed;

	void dummyReq(uint32_t handle);
	void clearDummyReq();
};

} /* namespace cahttp */

#endif /* CAHTTP_REQMAN_H_ */
