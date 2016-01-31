/*
 * HttpReq.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG
#include "flog.h"

#include "HttpReq.h"
#include "CaHttpUrlParser.h"
#include "StringPacketBuf.h"

namespace cahttp {

HttpReq::HttpReq() :
		mCnnIf(this) {
	mSvrIp = 0;
	mSvrPort = 80;
	mpCnn = nullptr;
	mCnnHandle = 0;
}

HttpReq::~HttpReq() {
}

int HttpReq::request(http_method method) {
	mReqMsg.setMethod(method);
	auto msg = mReqMsg.serialize();
	auto& url = mReqMsg.getUrl();
	if(mpCnn==nullptr) {
		mpCnn = new BaseConnection;
		mPropCnn.reset(mpCnn);
		if (mSvrIp == 0) {
			CaHttpUrlParser parser;
			parser.parse(url);
			if(parser.port != "") {
				mSvrPort = stoi(parser.port);
			}
			ali("resolving server ip, host=%s", parser.hostName);
			mSvrIp = get_ip_from_hostname(parser.hostName);
			ali("  ip=%s, port=%d", get_ipstr(mSvrIp), mSvrPort);
		}
		if (mSvrPort == 0) {
			mSvrPort = 80;
		}
		auto ret = mpCnn->connect(mSvrIp, mSvrPort);
		ald("connecting, ret=%d", ret);
	}


	if (mpCnn) {
		mReqMsg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
		string msgstr = mReqMsg.serialize();
		mCnnHandle = mpCnn->startSend(&mCnnIf);
		if (mCnnHandle) {
			sendPacket(move(msgstr));
			return 0;
		} else {
			ale("### Error:fail to get handle of connection");
			assert(0);
			return -1;
		}
	} else {
		ale("### no connection,...");
		assert(0);
		return -1;
	}

}

HttpReq::ReqCnnIf::ReqCnnIf(HttpReq* req) {
	mpReq = req;
}

HttpReq::ReqCnnIf::~ReqCnnIf() {
}

void HttpReq::ReqCnnIf::OnWritable() {
	mpReq->procWritable();
}

int HttpReq::sendPacket(const char* buf, size_t len) {
	auto ret = mpCnn->send(buf, len);
	if (ret > 0) {
		// send fail
		auto *pkt = new StringPacketBuf;
		pkt->setString(buf, len);
		mBufList.emplace_back(pkt);
		return -1;
	} else {
		return ret;
	}
}

int HttpReq::sendPacket(string&& s) {
	auto ret = mpCnn->send(s.data(), s.size());
	if (ret > 0) {
		// send fail
		auto *pkt = new StringPacketBuf;
		pkt->setString(move(s));
		mBufList.emplace_back(pkt);
		return -1;
	} else {
		return ret;
	}
}

void HttpReq::procWritable() {
	for (; mBufList.empty() == false;) {
		auto buf = mBufList.front()->getBuf();
		if (buf.first > 0) {
			auto ret = mpCnn->send(buf.second, buf.first);
			if (ret <= 0) {
				mBufList.pop_front();
			} else {
				break;
			}
		} else {
			ale("### Error: buf data size invalid, size=%lu", buf.first);
			assert(0);
			mBufList.pop_front();
			break;
		}
	}
}

} /* namespace cahttp */
