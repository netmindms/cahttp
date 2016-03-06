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
	void OnIdle() override;
	void OnDisconnected() override;
	void setRelLis(std::function<void(uint32_t)>);
	uint32_t getHandle() {
		return mHandle;
	}
private:
	uint32_t mHandle;
	std::function<void(uint32_t)> mLis;
	void setHandle(uint32_t handle) {
		mHandle = handle;
	}
};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDCONNECTION_H_ */
