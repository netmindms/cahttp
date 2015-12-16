/*
 * CaHttpReq.h
 *
 *  Created on: Apr 19, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPREQ_H_
#define SRC_CAHTTPREQ_H_

#include <ednio/EdFile.h>
#include <functional>
#include <string>
#include <bitset>
#include <ednio/EdNio.h>
#include "CaHttpCommon.h"
#include "CaHttpMsg.h"
#include "HttpBaseReadStream.h"
#include "HttpBaseWriteStream.h"
#include "HttpHeaderSet.h"
#include "HttpMsgStream.h"

using namespace edft;
namespace cahttp {
class CaHttpReq;
class HttpCnn;
class CaHttpReqMan;


class CaHttpReq {
friend class HttpCnn;
friend class CaHttpReqMan;
public:
	enum HRE {
		HTTP_RESP_HDR,
		HTTP_RESP_DATA,
		HTTP_REQ_DATA_UNDERRUN,
		HTTP_REQ_END,
	};

	typedef std::function<void (CaHttpReq&, HRE, int)> Lis;
	CaHttpReq();
	virtual ~CaHttpReq();
	int getMethod();
	int getRespCode();
	long getRespContentLen();
	int64_t getReqContentLen();
	CaHttpMsg& getReqMsg();
	CaHttpMsg& getRespMsg();
	void setOnListener(Lis lis);
	void setReqHdr(const std::string& name, const std::string &val);
	void setReqMsg(CaHttpMsg &&msg);
	void setReqContent(const std::string& data, const std::string& ctype);
	void setReqContent(const char* ptr, size_t len, const std::string& ctype);
	void setReqContent(HttpBaseReadStream *pstrm, const std::string& ctype);
	void setReqContent(size_t len, const std::string& ctype);
	void setUrl(const std::string& urlstr);
	void setRespDataStream(HttpBaseWriteStream *pstrm);
	void enableTransferChunked();
	HttpBaseWriteStream* getHttpRespStream();

//	int request(const std::string &method, const std::string& path, HdrList &&hdrlist, upHttpBaseReadStream sstrm, upHttpBaseWriteStream rstrm, Lis lis);
	int request(const std::string& method, const std::string& path, Lis lis);
	int request(const std::string& method);
	int request_get(const std::string& path, Lis lis);
	int request_post(const std::string& path, Lis lis);
	void setRemoteHostAddr(uint32_t ip, int port);
	void setRemoteHostAddr(const char *ip, int port);
	void setRemoteHostAddr(const std::string &ip, int port);
	void getRespData(std::string& respdata);
	void close();
	int clearRequest();
	const std::string& getRespData();
	int sendData(const char* ptr, size_t len);

private:
	int64_t mReqContentLen;
	int64_t mSendWriteCnt;
	CaHttpMsg mReqMsg;
	HttpMsgStream mReqMsgStrm;
	HttpBaseWriteStream *mpRespDataStrm;
	CaHttpMsg mRespMsg;
	std::string mRecvDataBuf;
	Lis mLis;
	uint32_t mHostIp;
	int mHostPort;
	HttpBaseReadStream *mpReqDataStrm;
	upHttpBaseReadStream mupDefReqDataStrm;
	CaHttpReqMan* mpReqMan;
	HttpCnn *mpCnn;
	std::unique_ptr<HttpCnn> mpuCnn;
	size_t mMaxRecvBufSize;

	std::bitset<8> mStatusFlag;
	std::string mChunkedBuf;

	void setReqMan(CaHttpReqMan* preqman);
	int writeTec(const char* ptr, size_t len);
};
}
#endif /* SRC_CAHTTPREQ_H_ */
