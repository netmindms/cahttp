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
	mMethod = HTTP_GET;
	mRespStatusCode = 0;
	mStatus.val = 0;
	mpClenHdr = nullptr;
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
	mRespStatusCode = 0;
}

void BaseMsg::setUrl(const std::string& urlstr) {
	mUrlStr = urlstr;
}

void BaseMsg::setContentType(const std::string& type) {
	setHdr(CAS::HS_CONTENT_TYPE, type);
}

void BaseMsg::setContentLen(int64_t len) {
	mContentLen = len;
	if(mContentLen>=0) {
		if(mpClenHdr == nullptr) {
			mpClenHdr = findHdr(CAS::HS_CONTENT_LEN);
		}
		if(mpClenHdr==nullptr) {
			mHdrList.emplace_back(CAS::HS_CONTENT_LEN, to_string(len));
			mpClenHdr = &(mHdrList.back());
		}
		mpClenHdr->second = to_string(len);
		if(mStatus.te) {
			removeHdr(CAS::HS_CONTENT_LEN);
		}
	}
}

void BaseMsg::removeHdr(const std::string& name) {
	for(auto itr=mHdrList.begin(); itr != mHdrList.end(); itr++) {
		if(!strcasecmp(name.data(), itr->first.data())) {
			mHdrList.erase(itr);
			break;
		}
	}
}

void BaseMsg::setHdr(const std::string& name, const std::string& val) {

	for(auto itr=mHdrList.begin(); itr != mHdrList.end(); itr++) {
		if(!strcasecmp(name.data(), itr->first.data())) {
			itr->second = val;
			return;
		}
	}

	addHdr(name, val);
}

std::string BaseMsg::serialize() {
	string encstr;
	if (mMsgType == MSG_TYPE_E::REQUEST) {
		if (mProtocolVer.empty()) {
			mProtocolVer = "HTTP/1.1";
		}
		encstr = string(http_method_str(mMethod)) + " " + mUrlStr + " "+mProtocolVer+"\r\n";
		ald("request line: %s", encstr);
	} else if (mMsgType == MSG_TYPE_E::RESPONSE) {
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


void BaseMsg::setTransferEncoding(bool te) {
	if(	mStatus.te == 0 && te) {
		addHdr(CAS::HS_TRANSFER_ENC, "chunked");
		mStatus.te = 1;
	} else if( mStatus.te == 1 && !te){
		if( mStatus.te == 1) {
			removeHdr(CAS::HS_TRANSFER_ENC);
		}
	}
}

std::pair<std::string, std::string>* BaseMsg::findHdr(const std::string& name) {
	for(auto &h: mHdrList) {
		if(!strcasecmp(h.first.c_str(), name.c_str())) {
			return &h;
		}
	}
	return nullptr;
}

} /* namespace cahttp */
