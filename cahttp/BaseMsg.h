/*
 * BaseMsg.h
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_BASEMSG_H_
#define CAHTTP_BASEMSG_H_

#include <memory>
#include <string>
#include <list>
#include "CaHttpCommon.h"
#include "http_parser.h"

namespace cahttp {

class BaseMsg {
private:
	union status_t {
		unsigned char val;
		struct {
			uint8_t c_len:1;
			uint8_t c_te:1;
			uint8_t c_ct:1;
			uint8_t te:1;
		};
		struct {
			uint8_t cache:2;
		};
	};
public:
	enum MSG_TYPE_E { REQUEST, RESPONSE };
	BaseMsg();
	BaseMsg(MSG_TYPE_E type);
	virtual ~BaseMsg();
	int64_t getContentLen();
	void setContentLen(int64_t len);
	bool getTransferEncoding();
	void setTransferEncoding(bool te);
	void setContentType(const std::string& type);
	const std::string& getContentType();

	int getMsgType() {
		return mMsgType;
	}
	void setMsgType(MSG_TYPE_E type) {
		mMsgType = type;
	}
	void setUrl(const char* ptr, size_t len);
	void setUrl(const std::string &urlstr);

	std::string& getUrl() {
		return mUrlStr;
	}

	void addHdr(const std::string& name, const std::string &val);
	void setHdr(const std::string& name, const std::string &val);
	void removeHdr(const std::string& name);
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
	void setProtocolVer(const std::string& protocl_ver) {
		mProtocolVer = protocl_ver;
	}




	std::string dumpHdr();
	void clear();
	std::string serialize();

private:
	int mRespStatusCode;
	std::string mProtocolVer;
	http_method mMethod;
	int64_t mContentLen;
	MSG_TYPE_E mMsgType;
	std::string mUrlStr;
	status_t mStatus;
	std::string* mpCtypeHdr;

	std::list<std::pair<std::string, std::string>> mHdrList;
	std::pair<std::string, std::string>* findHdr(const std::string& name);

};

typedef std::unique_ptr<BaseMsg> upBaseMsg;

} /* namespace cahttp */

#endif /* CAHTTP_BASEMSG_H_ */
