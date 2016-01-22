/*
 * CaHttpUrlCtrl.h
 *
 *  Created on: Apr 24, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPURLCTRL_H_
#define SRC_CAHTTPURLCTRL_H_

#include <memory>
#include <unordered_map>
#include <regex>
#include <list>

#include "CaHttpReq.h"
#include "CaHttpSvrReq.h"
#include "CaSimpleHeaderEnc.h"
#include "HttpStringReadStream.h"

namespace cahttp {
class ServCnn;
class CaHttpUrlCtrl {
	friend class ServCnn;
	friend class HttpServCnnCtx;
public:
	CaHttpUrlCtrl();
	virtual ~CaHttpUrlCtrl();
	virtual void OnHttpReqMsgHdr();
	virtual void OnHttpReqData(std::string &&data);
	virtual void OnHttpReqNonHttpData(const char* ptr, size_t len);
	virtual void OnHttpReqDataEnd();
	virtual void OnHttpReqMsg();
	virtual void OnHttpSendBufReady();
	virtual void OnHttpEnd();
	CaHttpSvrReq &getReq();
	void response(int status);
	void response(int status, const std::string& data, const std::string& content_type="plain/text");
	void response(int status, std::string&& data, std::string&& content_type);
	void response(int status, std::vector<hdrpair> &&hdrlist, HttpBaseReadStream *strm);
	const std::vector<std::string> &getUrlMatchStr();
	ServCnn& getConnection() { return *mCnn;};
	int getRespCode();
	std::string dump();
	int sendData(const char* ptr, size_t len);
	int sendString(const std::string& str);
	void addRespHdr(const std::string &name, const std::string &val);
	void setRespContent(const char* ptr, int64_t data_len, std::string&& ctype, bool tec=false);
	void setRespContent(const std::string &str, std::string&& ctype, bool tec=false);
	void setRespContent(HttpBaseReadStream *strm , int64_t len);
	const std::string& getReqHdr(const char* name);
	uint32_t getHandle();
	uint64_t getContentLen();
	const std::string& getReqUrlStr();
	const std::string& getReqData();
	void setReqDataStream(HttpBaseWriteStream *strm);

private:
	CaHttpSvrReq mReq;
	CaHttpMsg mRespMsg;
	ServCnn *mCnn;
	HttpStringReadStream mHdrStrm;
	HttpBaseWriteStream *mReqDataStrm;
	std::unique_ptr<HttpBaseWriteStream> mDefStrm;
	std::unique_ptr<HttpBaseReadStream> mpuSelfDataStrm;
	HttpBaseReadStream *mDataStrm;
	std::vector<std::string> mUrlMatchRes;
	int64_t mWriteCnt, mDataReadCnt;
	std::string mRespData;
	int64_t mRespContentLen;
	uint8_t mFlag;
	std::pair<const char*, int64_t> mChkPtr;
	std::unique_ptr<HttpStringReadStream> mChkStrm;
	int mRespCode;
	uint32_t mHandle;

	// request processing
	void procIncomingReqMsg(ServCnn* pcnn, CaHttpMsg &&msg, bool msgonly);
	void procInReqData(std::string &&data, bool dataend);

	void setUrlMatchResult(const std::smatch &match);
	void setConnection(ServCnn &cnn);
	std::pair<const char*, int64_t> getDataPtr();
	void consume(size_t len);
	void consumeData(size_t len);
	bool isStreamMode();
	bool isSendComp();
	int procDataLack(); // return 0: end ctrl, other: continue
	void setHandle(uint32_t handle);
	int writeTec(const char* ptr, size_t len);

};

typedef struct {
	std::regex ex;
	std::function<CaHttpUrlCtrl* ()> allocCtrl;
} url_regex_info;

typedef std::function<CaHttpUrlCtrl* ()> UrlCtrlAlloc;
typedef std::unique_ptr<CaHttpUrlCtrl> upCaHttpUrlCtrl;
typedef std::unordered_map<std::string, UrlCtrlAlloc> UrlMap;
typedef std::list<std::pair<std::regex, UrlCtrlAlloc> > UrlRegExMap;
}
#endif /* SRC_CAHTTPURLCTRL_H_ */
