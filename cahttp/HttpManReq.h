/*
 * HttpManReq.h
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_HTTPMANREQ_H_
#define CAHTTP_HTTPMANREQ_H_

#include "HttpReq.h"

namespace cahttp {
class ReqMan;

class HttpManReq: public HttpReq {
	friend class ReqMan;
public:
	HttpManReq();
	virtual ~HttpManReq();
	void close() override;
private:
	uint32_t mHandle;
	ReqMan* mpReqMan;

	void init(ReqMan& rm, uint32_t handle) {
		mpReqMan = &rm;
		mHandle = handle;
	};
};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPMANREQ_H_ */
