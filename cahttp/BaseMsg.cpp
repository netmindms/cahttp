/*
 * BaseMsg.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include <assert.h>
#include "BaseMsg.h"
#include "CaHttpCommon.h"
#include "flog.h"

using namespace std;

namespace cahttp {

BaseMsg::BaseMsg() {
	mMsgType = REQUEST;
	mContentLen = 0;
}

BaseMsg::~BaseMsg() {
	// TODO Auto-generated destructor stub
}

BaseMsg::BaseMsg(MSG_TYPE_E type) {
	BaseMsg();
	mMsgType = type;
}


void BaseMsg::setUrl(const char* ptr, size_t len) {
	mUrlStr.assign(ptr, len);
}

void BaseMsg::addHdr(const std::string& name, const std::string& val) {
	mHdrList.emplace_back(name, val);
}

std::string BaseMsg::dumpHdr() {
	string res;
	for(auto &hv: mHdrList) {
		res += hv.first+": "+hv.second+"\n";
	}
	return move(res);
}

void BaseMsg::clear() {
	mHdrList.clear();
	mUrlStr.clear();
	mContentLen = 0;
	mProtocolVer.clear();
	mParserFlag = 0;
}

std::string BaseMsg::serialize() {
	string encstr;
	if (mMsgType == BaseMsg::REQUEST) {
		if (mProtocolVer.empty()) {
			mProtocolVer = "HTTP/1.1";
		}
		encstr = string(http_method_str(mMethod)) + " " + mUrlStr + " "+mProtocolVer+"\r\n";
		ald("request line: %s", encstr);
	} else if (mMsgType == BaseMsg::RESPONSE) {
		if (mProtocolVer.empty()) {
			mProtocolVer = "HTTP/1.1";
		}
		encstr = string(mProtocolVer) + " " + to_string(mRespStatusCode) + " " + get_status_desc(mRespStatusCode) + "\r\n";
	} else {
		return "";
	}

	// header enc

	for (auto &h : mHdrList) {
		encstr += h.first + ": " + h.second + "\r\n";
	}
	encstr += "\r\n";

	return move(encstr);
}

} /* namespace cahttp */
