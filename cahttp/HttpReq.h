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
#include <memory>

#include "BaseConnection.h"
#include "BaseMsg.h"
#include "PacketBuf.h"

namespace cahttp {

class HttpReq {
	friend class ReqCnnIf;
public:
	enum Event {
		ON_MSG,
		ON_DATA,
		ON_END,
	};
	typedef std::function<void (Event)> Lis;
	HttpReq();
	virtual ~HttpReq();
	int request_get(const std::string& url, Lis lis);
	int request_post(const std::string& url, Lis lis);
	int request(http_method method, const char *pdata=nullptr, size_t data_len=0, const char* ctype=nullptr);
	int sendPacket(const char* buf, size_t len);
	int sendPacket(std::string&& s);
	int getRespStatus();
	int64_t getRespContentLen();
	void setReqContent(const std::string& data, const std::string& content_type);
	void close();
private:
	class ReqCnnIf: public BaseConnection::CnnIf {
	friend class HttpReq;
		ReqCnnIf(HttpReq* req);
		virtual ~ReqCnnIf();
		void OnWritable() override;
		void OnMsg(std::unique_ptr<BaseMsg> upmsg) override;
		void OnData(std::string&& data) override;
		void OnCnn(int cnnstatus) override;
		HttpReq* mpReq;
	};
//	CaHttpUrlParser mUrlParser;
	BaseMsg mReqMsg;
	std::unique_ptr<BaseMsg> mupRespMsg;
	BaseConnection *mpCnn;
	uint32_t mSvrIp;
	int mSvrPort;
	std::unique_ptr<BaseConnection> mPropCnn;
	uint32_t mCnnHandle;
	int64_t mRecvDataCnt;
	std::string mRecvDataBuf;

	ReqCnnIf mCnnIf;
	std::list<std::unique_ptr<PacketBuf>> mBufList;
	Lis mLis;

	int sendHttpMsg(std::string& msg);
	void procWritable();
	void procOnMsg(std::unique_ptr<BaseMsg> upmsg);
	void procOnData(std::string &data);
};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPREQ_H_ */
