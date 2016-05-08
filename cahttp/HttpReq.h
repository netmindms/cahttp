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

#include <ednio/EdNio.h>
#include "CaHttpCommon.h"
#include "BaseConnection.h"
#include "BaseMsg.h"
#include "PacketBuf.h"
#include "MsgTransmitter.h"
#include "MsgSender.h"
#include "SimpleCnn.h"

namespace cahttp {

class HttpCnnMan;

class HttpReq {
	friend class ReqMan;

private:
	union status_t {
		uint8_t val;
		struct {
			uint8_t used: 1;
			uint8_t te: 1;
			uint8_t se: 1; // sending end
			uint8_t fin: 1; // req finished
		};
	};
public:
	enum Event {
		ON_MSG,
		ON_DATA,
		ON_SEND,
		ON_END,
	};
	typedef std::function<void (Event, int err)> Lis;
	HttpReq();
	HttpReq(const HttpReq& that) = delete;
	virtual ~HttpReq();
	int request(BaseMsg& msg);
	int request_get(const std::string& url, Lis lis);
	int request_post(const std::string& url, Lis lis);
	int request(http_method method, const char* url, const char *pdata=nullptr, size_t data_len=0, const std::string& ctype=CAS::CT_TEXT_PLAIN, Lis lis=nullptr);
	int request(http_method method, const std::string& url, const std::string& data, const std::string& ctype, Lis lis=nullptr);
	int setContentFile(const char* path, const std::string& ctype);
	void addHeader(const std::string& name, const std::string& val) {
		mReqMsg.addHdr(name, val);
	};

	void setContentType(const std::string& ctype) {
		mReqMsg.setContentType(ctype);
	}

	int sendContent(const char* ptr, size_t len);
	SR sendData(const char* ptr, size_t len);
	int getRespStatus();
	int64_t getRespContentLen();
	void setContentLen(int64_t);
	void setReqContent(const std::string& data, const std::string& content_type);
	int setReqContentFile(const std::string& path, const std::string& content_type);
	void transferEncoding(bool te);
	void endData();
	virtual void close();
	std::string fetchData();
	inline void setReqContentType(const std::string& content_type) {
		mReqMsg.addHdr(cahttp::CAS::HS_CONTENT_TYPE, content_type);
	}
	inline void addReqHdr(const std::string& name, const std::string& val) {
		mReqMsg.addHdr(name, val);
	}
	void setOnListener(Lis lis) {
		mLis = lis;
	};
	void setContent(const char *ptr, size_t len, const std::string& ctype);
	inline ERR getError() {
		return mErr;
	}
	int setContentInfoFile(const char* path, const std::string& ctype);
	void setContentInfo(int64_t len, const std::string& ctype);
	int clear();
	int sendContentFile(const char* path);
	inline void reserveWrite() {
		mMsgTx.reserveWrite();
	}
	void setCnnMan(HttpCnnMan& cnnman) {
		mpCnnMan = &cnnman;
	}
private:
	status_t mStatus;
	BaseMsg mReqMsg;
	std::unique_ptr<BaseMsg> mupRespMsg;
	std::string mRecvDataBuf;
	std::shared_ptr<SimpleCnn> mpCnn;
	uint32_t mSvrIp;
	uint16_t mSvrPort;

	int64_t mRecvDataCnt;
	Lis mLis;
	ERR mErr;
	MsgSender mMsgTx;
	edft::EdTimer mRespTimer;
	uint16_t mRespTimeoutSec;
	HttpCnnMan* mpCnnMan;

	int procOnMsg();
	int procOnData();
	int procOnCnn(int status);
	void setBasicHeader(BaseMsg& msg, http_method method);


protected:
	void setConnection(BaseConnection* pcnn) {
		// TODO:
//		mpCnn = pcnn;
	}

};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPREQ_H_ */
