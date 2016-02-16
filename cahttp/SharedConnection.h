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
	virtual int send(uint32_t handle, const char* buf, size_t len) override;
	virtual void close();
private:
	std::list<std::pair<uint32_t, CnnIf*>> mSharedIfs;

	uint32_t mHandleSeed;
	uint32_t mhCurSend;

	class SharedCnnIf: public CnnIf {
		friend class SharedConnection;
		SharedCnnIf(SharedConnection *pcnn) { mpCnn = pcnn;};
		virtual ~SharedCnnIf(){};
		// if return is not 0, read loop exit
		virtual int OnWritable() override;
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg) override;
		virtual int OnData(std::string&& data) override;
		virtual int OnCnn(int cnnstatus) override;
		SharedConnection *mpCnn;
	};

	SharedCnnIf mCnnIf;
};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDCONNECTION_H_ */
