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

#include "CaHttpCommon.h"
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
		ON_SEND,
		ON_END,
	};
	typedef std::function<void (Event)> Lis;
	HttpReq();
	HttpReq(const HttpReq& that) = delete;
	virtual ~HttpReq();
	int request_get(const std::string& url, Lis lis);
	int request_post(const std::string& url, Lis lis);
	int request(http_method method, const char *pdata=nullptr, size_t data_len=0, const char* ctype=nullptr);
	int sendData(const char* ptr, size_t len);
	int sendPacket(const char* buf, size_t len);
	int sendPacket(std::string&& s);
	int getRespStatus();
	int64_t getRespContentLen();
	void setReqContent(const std::string& data, const std::string& content_type);
	int setReqContentFile(const std::string& path, const std::string& content_type);
	inline void setReqContentType(const std::string& content_type) {
		mReqMsg.addHdr(cahttp::CAS::HS_CONTENT_TYPE, content_type);
	}
	inline void addReqHdr(const std::string& name, const std::string& val) {
		mReqMsg.addHdr(name, val);
	}
	std::string fetchData();
	void transferEncoding(bool te);
	void endData();
	void close();
private:
	class ReqCnnIf: public BaseConnection::CnnIf {
	friend class HttpReq;
		ReqCnnIf(HttpReq& req);
		virtual ~ReqCnnIf();
		int OnWritable() override;
//		int OnMsg(std::unique_ptr<BaseMsg> upmsg) override;
//		int OnData(std::string&& data) override;
		int OnCnn(int cnnstatus) override;
		HttpReq& mReq;
	};

	class localrecvif: public BaseConnection::RecvIf {
		friend class HttpReq;
		localrecvif(HttpReq& r);
		virtual ~localrecvif();
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg) override;
		virtual int OnData(std::string&& data) override;
		HttpReq& mReq;
	};

	BaseMsg mReqMsg;
	std::unique_ptr<BaseMsg> mupRespMsg;
	BaseConnection *mpCnn;
	uint32_t mSvrIp;
	int mSvrPort;
	std::unique_ptr<BaseConnection> mPropCnn;
	uint32_t mCnnHandle;
	int64_t mSendDataCnt;
	int64_t mRecvDataCnt;
	std::string mRecvDataBuf;
	int64_t mReqContentLen;
	uint8_t mStatusFlag;

	ReqCnnIf mCnnIf;
	localrecvif mRecvIf;
	std::list<std::unique_ptr<PacketBuf>> mBufList;
	Lis mLis;

	int sendHttpMsg(std::string&& msg);
	int procWritable();
	int procOnMsg(std::unique_ptr<BaseMsg> upmsg);
	int procOnData(std::string &data);
	int procOnCnn(int status);
	void stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail);
};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPREQ_H_ */
