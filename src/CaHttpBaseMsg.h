/*
 * CaRtspMsg.h
 *
 *  Created on: Oct 5, 2015
 *      Author: root
 */

#ifndef EXTERNAL_CAHTTP_CARTSPMSG_H_
#define EXTERNAL_CAHTTP_CARTSPMSG_H_

#include <climits>
#include <vector>
#include <utility>
#include <string>
#include "http_parser.h"
#include "CaHttpUrlParser.h"
namespace cahttp {
class CaHttpBaseMsg {
	friend class CaHttpFrame;
	friend class CaRtspFrame;
#define CLEN_INVALID LLONG_MIN
public:
	CaHttpBaseMsg();
	virtual ~CaHttpBaseMsg();

	CaHttpBaseMsg(const CaHttpBaseMsg& other);

	CaHttpBaseMsg(CaHttpBaseMsg&& other) { // move constructor
		*this = std::move(other);
	};

	CaHttpBaseMsg& operator=(CaHttpBaseMsg&& other) {
		if(this != &other) {
			method = other.method;
			mUrlStr = std::move(other.mUrlStr);other.mUrlStr.clear();
			mUrlParser = std::move(other.mUrlParser);
			msgType = other.msgType;
			mContentLen = other.mContentLen;other.mContentLen=CLEN_INVALID;
			mData = std::move(other.mData);other.mData.clear();
			mHdrs = std::move(other.mHdrs);
			mProtocol = other.mProtocol;
		}
		return *this;
	}

	void addHdr(const char* name, const char *val);
	void addHdr(std::string &&name, std::string &&val);
	void setHdr(const char *name, std::string &&val);
	void setUrl(const char *ustr, size_t len);
	void setUrl(std::string &&urlstr);
	const std::string& getUrl();
	const std::string& getHdr(const std::string& name);
	void setRequest(http_method m);
	void setResponse(int code);
	void setData(std::string &&data);
	void setProtocol(MSG_PROTOCOL prt);
	int64_t getContentLen();
	void setContentLen(int64_t size);

	std::string fetchData();
	const std::string& getData();

	const char* getMethodStr();
	std::string serialize(bool body=false);
	void clear();

	const std::string& getUrlPath();
	const CaHttpUrlParser&   getUrlParser();

	std::string dumpHdr();

public:
	union {
		http_method method;
		int responseCode;
	};
private:
	typedef std::pair<std::string, std::string> _Header;
	int msgType; // 1==request, 2==response
	std::string mData;
	std::vector<_Header> mHdrs;
	int64_t mContentLen;
	std::string mUrlStr;
	MSG_PROTOCOL mProtocol;
	CaHttpUrlParser mUrlParser;
};
}
#endif /* EXTERNAL_CAHTTP_CARTSPMSG_H_ */
