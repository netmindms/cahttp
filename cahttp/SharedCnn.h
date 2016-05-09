/*
 * SharedCnn.h
 *
 *  Created on: Mar 20, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_SHAREDCNN_H_
#define CAHTTP_SHAREDCNN_H_
#include <memory>

#include "CaHttpCommon.h"
#include "BaseMsg.h"
#include "Transport.h"

namespace cahttp {
class HttpCnnMan;
class BaseConnection;
class SharedCnn : public Transport {
public:
	SharedCnn();
	virtual ~SharedCnn();
	void setHttpCnnMan(HttpCnnMan& cnnman);


//	virtual int connect(uint32_t ip, int port, int timeout, std::function<void(CH_E)> lis); // timeout 30 sec
	cahttp::SR send(const char* buf, size_t len) override;
	void sendEnd() override;
	void recvEnd() override;
	void close() override;
	void openSharedCnn(std::shared_ptr<BaseConnection> spcnn);
	void setOnListener(std::function<void(CHEVENT)> lis) {
			mLis = lis;
		}
	void reserveWrite();
	BaseMsg* fetchMsg() {
			return mRecvMsg.release();
		};
		std::string fetchData() {
			std::string s;
			s = move(mRecvData); mRecvData.clear();
			return move(s);
		}
private:
	uint32_t mHandle;
	HttpCnnMan* mpCnnMan;
	upBaseMsg mRecvMsg;
	std::string mRecvData;
	std::shared_ptr<BaseConnection> mpPipeCnn;
	uint32_t mRxCh;
	uint32_t mTxCh;
	std::function<void(CHEVENT)> mLis;
};

} /* namespace cahttp */

#endif /* CAHTTP_SHAREDCNN_H_ */
