/*
 * BaseMsg.h
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_BASEMSG_H_
#define CAHTTP_BASEMSG_H_

#include <string>
#include <list>
#include "http_parser.h"

namespace cahttp {

class BaseMsg {
public:
	enum MSG_TYPE_E { REQUEST, RESPONSE };
	BaseMsg();
	BaseMsg(MSG_TYPE_E type);
	virtual ~BaseMsg();
	int64_t getContentLenInt() {
		return mContentLen;
	}
	void setContentLenInt(int64_t len) {
		mContentLen = len;
	}
	int getMsgType() {
		return mMsgType;
	}
	void setMsgType(MSG_TYPE_E type) {
		mMsgType = type;
	}
	void setUrl(const char* ptr, size_t len);
	void addHdr(const std::string& name, const std::string &val);
	void setMethod(http_method method) {
		mMethod = method;
	}
	http_method getMethod() {
		return mMethod;
	}
	void setRespStatus(int status_code) {
		mRespStatusCode = status_code;
	}
	int getRespStatus() {
		return mRespStatusCode;
	}
	void setParserFlag(uint8_t flag) {
		mParserFlag = flag;
	}
	void setProtocolVer(const std::string& protocl_ver) {
		mProtocolVer = protocl_ver;
	}
	std::string dumpHdr();
	void clear();
	std::string serialize();

private:
	int mRespStatusCode;
	std::string mProtocolVer;
	uint8_t mParserFlag;
	http_method mMethod;
	int64_t mContentLen;
	MSG_TYPE_E mMsgType;
	std::string mUrlStr;
	std::list<std::pair<std::string, std::string>> mHdrList;
};

} /* namespace cahttp */

#endif /* CAHTTP_BASEMSG_H_ */
