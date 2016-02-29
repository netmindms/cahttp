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
	assert(sizeof(status_t)==1);
	mMsgType = REQUEST;
	mContentLen = 0;
	mMethod = HTTP_GET;
	mRespStatusCode = 0;
	mStatus.val = 0;
	mpCtypeHdr = nullptr;
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
//	mpClenHdr = nullptr;
	mStatus.val = 0;
}

void BaseMsg::setUrl(const std::string& urlstr) {
	mUrlStr = urlstr;
}

const std::string& BaseMsg::getContentType() {
	static std::string nullstr;
	if(mStatus.c_ct) {
		if(mpCtypeHdr) {
			return mpCtypeHdr->second;
		} else {
			return nullstr;
		}
	} else {

	}
}

void BaseMsg::setContentType(const std::string& type) {
	if(mStatus.c_ct) {
		if(mpCtypeHdr) {
			mpCtypeHdr->second = type;
		} else {
			mHdrList.emplace_back(CAS::HS_CONTENT_TYPE, type);
			mpCtypeHdr = &(mHdrList.back());
		}
	} else {
		mHdrList.emplace_back(CAS::HS_CONTENT_TYPE, type);
		mpCtypeHdr = &(mHdrList.back());
	}
	mStatus.c_ct = 1;
}

#if 0
void BaseMsg::setContentLen(int64_t len) {
#if 1
	removeHdr(CAS::HS_TRANSFER_ENC);
	setHdr(CAS::HS_CONTENT_LEN, to_string(len));
	mContentLen = len;
#else
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
		if(mpTeHdr) {
			removeHdr(CAS::HS_TRANSFER_ENC);
			mpTeHdr = nullptr;
		}
	}
#endif
}
#endif

void BaseMsg::addHdr(const std::string& name, const std::string& val) {
	mStatus.cache = 0;
	mHdrList.emplace_back(name, val);
}

void BaseMsg::setHdr(const std::string& name, const std::string& val) {
	mStatus.cache = 0;
	for(auto itr=mHdrList.begin(); itr != mHdrList.end(); itr++) {
		if(!strcasecmp(name.data(), itr->first.data())) {
			itr->second = val;
			return;
		}
	}
	addHdr(name, val);
}

void BaseMsg::removeHdr(const std::string& name) {
	mStatus.cache = 0;
	for(auto itr=mHdrList.begin(); itr != mHdrList.end(); itr++) {
		if(!strcasecmp(name.data(), itr->first.data())) {
			mHdrList.erase(itr);
			break;
		}
	}
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



std::pair<std::string, std::string>* BaseMsg::findHdr(const std::string& name) {
	for(auto &h: mHdrList) {
		if(!strcasecmp(h.first.c_str(), name.c_str())) {
			return &h;
		}
	}
	return nullptr;
}


int64_t BaseMsg::getContentLen() {
	if(mStatus.c_len) {
		return mContentLen;
	} else {
		mStatus.c_len = 1;
		auto *phd = findHdr(CAS::HS_CONTENT_LEN);
		if(phd) {
			mContentLen = stol(phd->second);
		} else {
			mContentLen = 0;
		}
		return mContentLen;
	}
}

void BaseMsg::setContentLen(int64_t len) {
	setTransferEncoding(false);
	setHdr(CAS::HS_CONTENT_LEN, std::to_string(len));
	mStatus.c_len = 1;
	mContentLen = len;
}

bool BaseMsg::getTransferEncoding() {
	if(mStatus.c_te) {
		return mStatus.te;
	} else {
		mStatus.c_te = 1;
		auto *phdr = findHdr(CAS::HS_TRANSFER_ENC);
		if(phdr && phdr->second=="chunked") {
			mStatus.te = 1;
		} else {
			mStatus.te = 0;
		}
		return mStatus.te;
	}
};

void BaseMsg::setTransferEncoding(bool te) {
	mStatus.c_te = 1;
	mStatus.te = te;
	if(te) {
		removeHdr(CAS::HS_CONTENT_LEN);
		setHdr(CAS::HS_TRANSFER_ENC, "chunked");
		mContentLen=0;
		mStatus.c_len=1;
	} else {
		removeHdr(CAS::HS_TRANSFER_ENC);
	}

};

} /* namespace cahttp */

