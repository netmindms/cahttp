/*
 * SharedConnection.h
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_SHAREDCONNECTION_H_
#define CAHTTP_SHAREDCONNECTION_H_

#include "BaseConnection.h"

namespace cahttp {

class SharedConnection: public BaseConnection {
	friend class ReqMan;
public:
	SharedConnection();
	virtual ~SharedConnection();
	inline uint32_t ip() {
		return mSvrIp;
	}
	inline int port() {
		return mSvrPort;
	}
private:
	uint32_t mHandle;
	uint32_t mSvrIp;
	int mSvrPort;

	void setHandle(uint32_t handle) {
		mHandle = handle;
	}
};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDCONNECTION_H_ */
