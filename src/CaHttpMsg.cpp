/*
 * CaHttpMsg.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_INFO
#include <climits>
#include <ednio/EdConst.h>
#include "flog.h"
#include "CaHttpCommon.h"
#include "CaHttpMsg.h"
namespace cahttp {
CaHttpMsg::CaHttpMsg() {
	mMsgStatus = 0;
	mMsgClass = 0;
	mContentLen = 0;
	mMsgFlag = 0;
}

CaHttpMsg::~CaHttpMsg() {
}
//
//void CaHttpMsg::setHdr(string&& name, string&& val) {
//	for (auto &h : mHdrList) {
//		if (h.first == name) {
//			h.second.push_back(move(val));
//			return;
//		}
//	}
//
//	mHdrList.push_back( { move(name), { move(val) } });
//}

//void CaHttpMsg::setHdr(string&& name, const string& val) {
//	string tval = val;
//	setHdr(move(name), move(tval));
//}

void CaHttpMsg::setHdr(const string &name, const string &val) {
//	setHdr(string(name), string(val));
	for (auto &h : mHdrList) {
		if (h.first == name) {
			h.second.push_back(val);
			return;
		}
	}

	mHdrList.push_back( { move(name), { move(val) } });
}

string CaHttpMsg::dumpHdr() {
	string ds;
	for (auto &h : mHdrList) {
		ds += h.first + ": ";
		for (auto &v : h.second) {
			ds += v + ", ";
		}
		ds.pop_back();
		ds.pop_back();
		ds += "\n";
	}
	return move(ds);
}

void CaHttpMsg::clear() {
	mMsgClass = 0;
	mMsgStatus = 0;
	mHdrList.clear();
}

int CaHttpMsg::status() const {
	return mMsgStatus;
}

void CaHttpMsg::setStatus(int st) {
	mMsgClass = 2;
	mMsgStatus = st;
}

const string& CaHttpMsg::getUrlStr() const {
	return mUrl;
}

const string& CaHttpMsg::getRespDesc() const {
	return mRespDesc;
}

http_method CaHttpMsg::getMethod() const {
	return mReq;
}

int CaHttpMsg::getRespCode() const {
	return mMsgStatus;
}

void CaHttpMsg::setResponse(int code) {
	mMsgClass = 2;
	mMsgStatus = code;
}

void CaHttpMsg::setMethod(http_method method) {
	mMsgClass = 1;
	mReq = method;
}

void CaHttpMsg::setMethod(const string& method) {
	mMsgClass = 1;
	mMethodStr = method;
}

void CaHttpMsg::setPath(const string& path) {
	mUrl = path;
}

const string& CaHttpMsg::getHdrOne(const string& name) const {
	for (auto &h : mHdrList) {
		if (name == h.first) {
			return h.second[0];
		}
	}
	return edft::NULL_STR;
}

string CaHttpMsg::serialize() {
	string encstr;
	if (mMsgClass == 1) {
		if (mProtocolVer.empty()) {
			mProtocolVer = "HTTP/1.1";
		}
		if (mMethodStr.empty()) {
			encstr = METHOD_STR(mReq)+ " " + mUrl + " "+mProtocolVer+"\r\n";
		} else {
			encstr = mMethodStr + " " + mUrl + " "+mProtocolVer+"\r\n";
		}
		ald("request line: %s", encstr);
	} else if (mMsgClass == 2) {
		if (mProtocolVer.empty()) {
			mProtocolVer = "HTTP/1.1";
		}
		encstr = mProtocolVer + " " + to_string(mMsgStatus) + " " + get_status_desc(mMsgStatus) + "\r\n";
	} else {
		return "";
	}

	// header enc

	for (auto &h : mHdrList) {
		encstr += h.first + ": ";
		for (auto &v : h.second) {
			encstr += v + ", ";
		}
		encstr.pop_back();
		encstr.pop_back();
		encstr += "\r\n";
	}
	encstr += "\r\n";

	return move(encstr);
}

bool CaHttpMsg::IsHdr(const string& name) {
	for (auto &h : mHdrList) {
		if (name == h.first) {
			return true;
		}
	}
	return false;
}

void CaHttpMsg::setUrl(const char* buf, size_t len) {
	mUrl.assign(buf, len);
}

int64_t CaHttpMsg::getContentLenInt() const {
//	for(auto& h: mHdrList) {
//		if(h.first == "Content-Length") {
//			return stol(h.second[0]);
//		}
//	}

	return mContentLen;
}

void CaHttpMsg::setHdrList(HdrList&& hdrs) {
	mHdrList = move(hdrs);
}

void CaHttpMsg::setContentLenInt(size_t len) {
	mContentLen = len;
}

int CaHttpMsg::getMsgClass() const {
	return mMsgClass;
}

void CaHttpMsg::setMsgFlag(uint32_t flag) {
	mMsgFlag = flag;
}

uint32_t CaHttpMsg::getMsgFlag() const {
	return mMsgFlag;
}

bool CaHttpMsg::isChunked() {
	return (mMsgFlag & F_CHUNKED);
}

void CaHttpMsg::setProtocolVer(const char* protocol) {
	mProtocolVer = protocol;
}

const string& CaHttpMsg::getProtocolVer() const {
	return mProtocolVer;
}
}
