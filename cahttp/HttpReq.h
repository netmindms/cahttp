/*
 * HttpReq.h
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_HTTPREQ_H_
#define CAHTTP_HTTPREQ_H_

#include <string>
#include <list>

#include "BaseConnection.h"
#include "BaseMsg.h"
#include "PacketBuf.h"

namespace cahttp {

class HttpReq {
	friend class ReqCnnIf;
public:
	HttpReq();
	virtual ~HttpReq();
	int request(http_method method);
	int sendPacket(const char* buf, size_t len);
	int sendPacket(std::string&& s);
private:
	class ReqCnnIf: public BaseConnection::CnnIf {
	friend class HttpReq;
		ReqCnnIf(HttpReq* req);
		virtual ~ReqCnnIf();
		void OnWritable() override;
		HttpReq* mpReq;
	};
//	CaHttpUrlParser mUrlParser;
	BaseMsg mReqMsg;
	BaseConnection *mpCnn;
	uint32_t mSvrIp;
	int mSvrPort;
	std::unique_ptr<BaseConnection> mPropCnn;
	uint32_t mCnnHandle;

	ReqCnnIf mCnnIf;
	std::list<std::unique_ptr<PacketBuf>> mBufList;
	void procWritable();
};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPREQ_H_ */
