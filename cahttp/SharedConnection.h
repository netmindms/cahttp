/*
 * SharedConnection.h
 *
 *  Created on: Feb 15, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_SHAREDCONNECTION_H_
#define CAHTTP_SHAREDCONNECTION_H_

#include <utility>
#include <unordered_map>
#include <stdint.h>
#include "BaseConnection.h"
namespace cahttp {

class SharedConnection: public BaseConnection {
public:
	SharedConnection();
	virtual ~SharedConnection();
	virtual uint32_t startSend(CnnIf* pif) override;
	virtual void endSend(uint32_t handle) override;
	virtual cahttp::SEND_RESULT send(uint32_t handle, const char* buf, size_t len) override;
	virtual void close();
private:
	std::list<std::pair<uint32_t, CnnIf*>> mSharedIfs;

	uint32_t mHandleSeed;
	uint32_t mhCurSend;

};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDCONNECTION_H_ */
