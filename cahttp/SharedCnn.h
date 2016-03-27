/*
 * SharedCnn.h
 *
 *  Created on: Mar 20, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_SHAREDCNN_H_
#define CAHTTP_SHAREDCNN_H_
#include <memory>
#include "BaseCnn.h"

namespace cahttp {
class HttpCnnMan;
class BaseConnection;
class SharedCnn : public BaseCnn {
public:
	SharedCnn();
	virtual ~SharedCnn();
	void setHttpCnnMan(HttpCnnMan& cnnman);


	virtual int connect(uint32_t ip, int port, int timeout, std::function<void(CH_E)> lis); // timeout 30 sec
	virtual cahttp::SR send(const char* buf, size_t len);
	virtual void sendEnd();
	virtual void recvEnd();
	virtual void close();
	void openSharedCnn(std::shared_ptr<BaseConnection> spcnn);
private:
	uint32_t mHandle;
	HttpCnnMan* mpCnnMan;
	std::shared_ptr<BaseConnection> mpPipeCnn;
	uint32_t mRxCh;
	uint32_t mTxCh;
};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDCNN_H_ */
