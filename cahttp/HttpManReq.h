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

class HttpManReq: public HttpReq {
public:
	HttpManReq();
	virtual ~HttpManReq();
private:
	uint32_t mHandle;
};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPMANREQ_H_ */
