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
//#include "RingBufReadStream.h"
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
	void response(int status, vector<hdrpair> &&hdrlist, upHttpBaseReadStream strm);
	const vector<std::string> &getUrlMatchStr();
	ServCnn& getConnection() { return *mCnn;};
	int getRespCode();
	std::string dump();
	int sendData(const char* ptr, size_t len);
	int sendString(const string& str);
	void addRespHdr(const string &name, const string &val);
	void setRespContent(const char* ptr, int64_t data_len, std::string&& ctype, bool tec=false);
	void setRespContent(const string &str, std::string&& ctype, bool tec=false);
	void setRespContent(upHttpBaseReadStream strm , int64_t len);
	const string& getReqHdr(const char* name);
	uint32_t getHandle();
	uint64_t getContentLen();
	const string& getReqUrlStr();
	const string& getReqData();
	void setReqDataStream(HttpBaseWriteStream *strm);

private:
	CaHttpSvrReq mReq;
	CaHttpMsg mRespMsg;
	ServCnn *mCnn;
	HttpStringReadStream mHdrStrm;
	HttpBaseWriteStream *mReqDataStrm;
	unique_ptr<HttpBaseWriteStream> mDefStrm;
	unique_ptr<HttpBaseReadStream> mDataStrm;
	vector<std::string> mUrlMatchRes;
	int64_t mWriteCnt, mDataReadCnt;
	string mRespData;
	int64_t mRespContentLen;
	uint8_t mFlag;
	pair<const char*, int64_t> mChkPtr;
	unique_ptr<HttpStringReadStream> mChkStrm;
	int mRespCode;
	uint32_t mHandle;

	// request processing
	void procIncomingReqMsg(ServCnn* pcnn, CaHttpMsg &&msg, bool msgonly);
	void procInReqData(string &&data, bool dataend);

	void setUrlMatchResult(const smatch &match);
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
	function<CaHttpUrlCtrl* ()> allocCtrl;
} url_regex_info;

typedef function<CaHttpUrlCtrl* ()> UrlCtrlAlloc;
typedef unique_ptr<CaHttpUrlCtrl> upCaHttpUrlCtrl;
typedef unordered_map<std::string, UrlCtrlAlloc> UrlMap;
typedef list<pair<regex, UrlCtrlAlloc> > UrlRegExMap;
}
#endif /* SRC_CAHTTPURLCTRL_H_ */
