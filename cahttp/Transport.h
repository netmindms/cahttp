/*
 * Transport.h
 *
 *  Created on: May 10, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_TRANSPORT_H_
#define CAHTTP_TRANSPORT_H_

#include "CaHttpCommon.h"

namespace cahttp {

class Transport {
public:
	Transport(){};
	virtual ~Transport() {};
	virtual cahttp::SR send(const char* buf, size_t len)=0;
	virtual void sendEnd()=0;
	virtual void recvEnd()=0;
	virtual void close()=0;
};

}


#endif /* CAHTTP_TRANSPORT_H_ */
