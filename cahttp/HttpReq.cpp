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
#include "CaHttpCommon.h"

namespace cahttp {

HttpReq::HttpReq() :
		mCnnIf(this) {
	mSvrIp = 0;
	mSvrPort = 80;
	mpCnn = nullptr;
	mCnnHandle = 0;
	mRecvDataCnt = 0;
}

HttpReq::~HttpReq() {
}

int HttpReq::request(http_method method, const char *pdata, size_t data_len, const char* ctype) {
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
		if(pdata) {
			if(ctype) mReqMsg.addHdr(CAS::HS_CONTENT_TYPE, ctype);
			mReqMsg.addHdr(CAS::HS_CONTENT_LEN, to_string(data_len));
		}
		string msgstr = mReqMsg.serialize();
		if(pdata) {
			msgstr.append(pdata, data_len);
		}
		mCnnHandle = mpCnn->startSend(&mCnnIf);
		if (mCnnHandle) {
			sendHttpMsg(msgstr);
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

int HttpReq::request_get(const std::string& url, Lis lis) {
	mLis = lis;
	mReqMsg.setUrl(url);
	return request(HTTP_GET);
}

void HttpReq::procWritable() {
	ald("proc writable, buf list cnt=%d", mBufList.size());
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

void HttpReq::ReqCnnIf::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	mpReq->procOnMsg(move(upmsg));
}

int HttpReq::getRespStatus() {
	if(mupRespMsg) {
		return mupRespMsg->getRespStatus();
	} else {
		ale("### Error: no response message");
		assert(0);
		return 0;
	}
}

int64_t HttpReq::getRespContentLen() {
	return mupRespMsg->getContentLenInt();
}

void HttpReq::procOnMsg(std::unique_ptr<BaseMsg> upmsg) {
	mupRespMsg = move(upmsg);
	assert(mLis);
	mLis(ON_MSG);
	if(mupRespMsg->getContentLenInt()==0) {
		mLis(ON_END);
	}
}

void HttpReq::ReqCnnIf::OnData(std::string&& data) {
	mpReq->procOnData(data);
}

void HttpReq::ReqCnnIf::OnCnn(int cnnstatus) {
}

void HttpReq::close() {
	if(mPropCnn) {
		mPropCnn->close();
		mPropCnn.reset();
	}
}

void HttpReq::setReqContent(const std::string& data, const std::string& content_type) {
	mReqMsg.addHdr(CAS::HS_CONTENT_TYPE, content_type);
	mReqMsg.addHdr(CAS::HS_CONTENT_LEN, to_string(data.size()));
	StringPacketBuf *pbuf = new StringPacketBuf;
	pbuf->setString(data.data(), data.size());
	mBufList.emplace_back(pbuf);
}

int HttpReq::request_post(const std::string& url, Lis lis) {
	mLis = lis;
	mReqMsg.setUrl(url);
	return request(HTTP_POST);
}

int HttpReq::sendHttpMsg(std::string& msg) {
	auto ret = mpCnn->send(msg.data(), msg.size());
	if (ret > 0) {
		// send fail
		auto *pkt = new StringPacketBuf;
		pkt->setString(move(msg));
		mBufList.emplace_front(pkt);
		return -1;
	} else {
		return ret;
	}
}

void HttpReq::procOnData(std::string& data) {
	mRecvDataCnt += data.size();
	if(mRecvDataBuf.empty()==true) {
		mRecvDataBuf = move(data);
	} else {
		mRecvDataBuf.append(data);
	}
	mLis(ON_DATA);
	if(mRecvDataCnt == mupRespMsg->getContentLenInt()) {
		mLis(ON_END);
	}
}

} /* namespace cahttp */
