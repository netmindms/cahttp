/*
 * CaHttpMsg.h
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPMSG_H_
#define SRC_CAHTTPMSG_H_

#include <vector>
#include <string>

#include "CaHttpCommon.h"
#include "http_parser.h"

namespace cahttp {
class CaHttpMsg
{
public:
	CaHttpMsg();
	/* TODO: check if needed
	CaHttpMsg(const CaHttpMsg& msg) {

	};
	*/
	CaHttpMsg(CaHttpMsg &&other) { // move constructor
		*this = move(other);
	};

	CaHttpMsg& operator=(CaHttpMsg &&other) { // move operator
		if(&other != this)
		{
			mReq = other.mReq;
//			mRespCode = other.mRespCode;
			mUrl = std::move(other.mUrl);
			mHdrList = std::move(other.mHdrList);
			mMsgStatus = other.mMsgStatus;
			mMsgClass = other.mMsgClass;
			mContentLen = other.mContentLen;
			mProtocolVer = std::move(other.mProtocolVer);

			other.mReq = HTTP_GET;
			other.mUrl.clear();
			other.mHdrList.clear();
			other.mMsgStatus = 0;
			other.mContentLen = 0;
			other.mProtocolVer.clear();
			other.mMsgClass = 0;
		}
		return *this;
	};

	virtual ~CaHttpMsg();

	void setHdr(const std::string &name, const std::string &val);
//	void setHdr(std::string &&name, std::string &&val);
//	void setHdr(std::string &&name, const std::string &val);
	void setContentLenInt(size_t len);
	const std::string& getHdrOne(const std::string& name) const;
	std::string dumpHdr();
	void clear();
	int status() const;
	void setStatus(int st);
	const std::string& getUrlStr() const;
	const std::string& getRespDesc() const;
	http_method getMethod() const;
	int getRespCode() const;
	void setResponse(int code);
	void setMethod(http_method method);
	void setMethod(const std::string &method);
	void setPath(const std::string &path);
	bool IsHdr(const std::string &name);

	int64_t getContentLenInt() const;
	void setUrl(const char* buf, size_t len);
	std::string serialize();
	void setHdrList(HdrList&& hdrs);
	int getMsgClass() const;
	void setMsgFlag(uint32_t flag);
	uint32_t getMsgFlag() const;
	bool isChunked();
	void setProtocolVer(const char* protocol);
	const std::string& getProtocolVer() const;
private:
	union {
			http_method mReq;
			int mMsgStatus;
	};
	std::string mUrl;
	std::string mRespDesc;
	HdrList mHdrList;
	int mMsgClass; // 0==empty, 1==request, 2==resp
	std::string mMethodStr;
	int64_t mContentLen;
	uint32_t mMsgFlag;
	std::string mProtocolVer;
};
}
#endif /* SRC_CAHTTPMSG_H_ */
