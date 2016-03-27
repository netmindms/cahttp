/*
 * HttpReq.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "HttpReq.h"
#include "FilePacketBuf.h"
#include "CaHttpUrlParser.h"
#include "StringPacketBuf.h"
#include "CaHttpCommon.h"
#include "BytePacketBuf.h"
#include "TEEndPacketBuf.h"
#include "BaseMsg.h"
#include "BaseCnn.h"

#include "HttpCnnMan.h"
#include "ext/nmdutil/FileUtil.h"
#include "ext/nmdutil/etcutil.h"
#include "ext/nmdutil/strutil.h"

#include "flog.h"

using namespace std;
using namespace cahttpu;

namespace cahttp {

HttpReq::HttpReq() {
	mSvrIp = 0;
	mSvrPort = 80;
	mpCnn = nullptr;
	mRecvDataCnt = 0;
	mStatus.val = 0;
	mErr = ERR::eNoErr;
	mRespTimeoutSec = 20; // 20 sec
	mpCnnMan = nullptr;
}

HttpReq::~HttpReq() {
}

int HttpReq::request(BaseMsg &msg) {
	if(mStatus.used) {
		return -1;
	}
	mStatus.used = 1;
	uint32_t s_ip=0;
	int s_port=80;

	auto& url = msg.getUrl();

	if (mSvrIp == 0) {
		CaHttpUrlParser parser;
		parser.parse(url);
		if(parser.port != "") {
			s_port = stoi(parser.port);
		}
		ald("resolving server ip, host=%s", parser.hostName);
		s_ip = get_ip_from_hostname(parser.hostName);
		ald("  ip=%s, port=%d", get_ipstr(s_ip), s_port);
	}

	int ret;
	if(mpCnn) {
		auto addr = mpCnn->getRmtAddr();
		if(addr.first != s_ip || addr.second != s_port) {
			mpCnn.reset();
		}
	}

	if(!mpCnn) {
		if(!mpCnnMan) {
			mpCnn.reset(new BaseCnn);
			ret = mpCnn->connect(s_ip, s_port, 30000, nullptr);
		} else {
			auto cnn = mpCnnMan->connect(s_ip, s_port);
			mpCnn = move(cnn.first);
			ret = cnn.second;
		}
	}

	mpCnn->setOnListener([this](BaseCnn::CH_E evt) ->int {
		alv("rx ch event=%d", (int)evt);
		if(evt == BaseCnn::CH_E::CH_MSG) {
			return procOnMsg();
		} else if(evt == BaseCnn::CH_E::CH_DATA) {
			return procOnData();
		} else if(evt == BaseCnn::CH_E::CH_CLOSED) {
			if(!mStatus.fin) {
				alw("*** request early terminated");
				mRespTimer.kill();
				mErr = ERR::eEarlyDisconnected;
				mStatus.fin = 1;
				//						mMsgTx.close();
				mLis(ON_END, mErr);
			}
			//					mMsgTx.close();
			return 0;
		} else if(evt == BaseCnn::CH_E::CH_WRITABLE) {
			auto r = mMsgTx.procOnWritable();
			if(r == MsgSender::kMsgDataNeeded) {
				mLis(ON_SEND, 0);
			}
		} else {
			assert(0);
			return 1;
		}
	});
	ald("connecting, ret=%d", ret);
	mMsgTx.open(*mpCnn);
	if(ret) {
		mRespTimer.set(mRespTimeoutSec*1000, 0, [this]() {
			alw("*** response timeout");
			mRespTimer.kill();
			mStatus.fin = 1;
			mErr = ERR::eNoResponse;
			mLis(ON_END, mErr);
		});
	}
	if (mpCnn) {
		mMsgTx.sendMsg(msg);
	} else {
		ale("### no connection,...");
		assert(0);
		return -1;
	}

}

int HttpReq::request_get(const std::string& url, Lis lis) {
	mLis = lis;
	mReqMsg.setUrl(url);
	setBasicHeader(mReqMsg, HTTP_GET);
	mReqMsg.setContentLen(0);
	return request(mReqMsg);
}


int HttpReq::request_post(const std::string& url, Lis lis) {
	mLis = lis;
	mReqMsg.setUrl(url);
	setBasicHeader(mReqMsg, HTTP_POST);
	return request(mReqMsg);
}

int HttpReq::getRespStatus() {
	if(mupRespMsg) {
		return mupRespMsg->getRespStatus();
	} else {
		alw("*** no response message");
		return 0;
	}
}

int64_t HttpReq::getRespContentLen() {
	return mupRespMsg->getContentLen();
}

int HttpReq::procOnMsg() {
	mRespTimer.kill();
	mupRespMsg.reset( mpCnn->fetchMsg() );
	assert(mLis);
	mLis(ON_MSG, 0);
	return 0;
}

void HttpReq::close() {
	mRespTimer.kill();
	if(mpCnn) {
		mpCnn->close();
		mSvrIp = 0;
		mSvrPort = 0;
		mStatus.val = 0;
		clear();
	}
}

std::string HttpReq::fetchData() {
	return move(mRecvDataBuf);
}

int HttpReq::procOnData() {
	auto data = mpCnn->fetchData();
	alv("proc on data, size=%ld", data.size());
	if(data.size()==0) {
		ald("empty data, consider as message end signal,");
		mStatus.fin = 1;
		mpCnn->recvEnd();
		mLis(ON_END, 0);
		return 0;
	}
	mRecvDataCnt += data.size();
	if(mRecvDataBuf.empty()==true) {
		mRecvDataBuf = move(data);
	} else {
		mRecvDataBuf.append(data);
	}
	mLis(ON_DATA, 0);
	return 0;
}

int HttpReq::procOnCnn(int status) {
	if(status==0) {
		ald("disconnected,...");
		if(!mStatus.fin) {
			ald("*** request terminated prematurely");
//			mStatus.fin = 1;
			mMsgTx.close();
			mErr = ERR::eEarlyDisconnected;
//			mpCnn.reset();
			mLis(ON_END, mErr);
			return 1;
		}
	}
	return 0;
}

void HttpReq::transferEncoding(bool te) {
	mReqMsg.setTransferEncoding(te);
}

void HttpReq::endData() {
	mMsgTx.endData();
}

void HttpReq::setBasicHeader(BaseMsg& msg, http_method method) {
	msg.setMsgType(BaseMsg::MSG_TYPE_E::REQUEST);
	msg.setMethod(method);
	msg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
}

int HttpReq::request(http_method method, const char* url, const char* pdata, size_t data_len, const std::string& ctype, Lis lis) {
	if(lis) mLis = lis;
	mReqMsg.setUrl(url);
	setBasicHeader(mReqMsg, method);
	if(data_len && !mReqMsg.getTransferEncoding()) {
		mReqMsg.setContentLen(data_len);
	}
	mReqMsg.setContentType(ctype);
	int r=0;
	r = request(mReqMsg);
	if(!r) {
		if(pdata && data_len>0) {
			r = mMsgTx.sendContent(pdata, data_len);
		}
	}

	return r;
}

SR HttpReq::sendData(const char* ptr, size_t len) {
	return mMsgTx.sendData(ptr, len, false);
}

int HttpReq::sendContent(const char* ptr, size_t len) {
	return mMsgTx.sendContent(ptr, len);
}

void HttpReq::setContentLen(int64_t longInt) {
	mReqMsg.setTransferEncoding(false);
	mReqMsg.setContentLen(longInt);
}

int HttpReq::clear() {
	if(mStatus.used) {
		if(mStatus.fin==1) {
			mStatus.val = 0;
			mReqMsg.clear();
			mupRespMsg.reset();
			mRecvDataBuf.clear();
			mRecvDataCnt=0;
			mErr = ERR::eNoErr;
			mMsgTx.clear();
			return 0;
		} else {
			ale("### clearing not available in request active state");
			return -1;
		}
	} else {
		return 0;
	}
}

int HttpReq::request(http_method method, const std::string& url, const std::string& data, const std::string& ctype, Lis lis) {
	return request(method, url.c_str(), data.c_str(), data.size(), ctype, lis);
}


int HttpReq::setContentInfoFile(const char* path, const std::string& ctype) {
	struct stat st;
	auto r = stat(path, &st);
	if(!r) {
		setContentInfo(st.st_size, ctype);
		return 0;
	} else {
		return -1;
	}
}

void HttpReq::setContentInfo(int64_t len, const std::string& ctype) {
	if(mReqMsg.getTransferEncoding()==false) {
		mReqMsg.setContentLen(len);
	}
	mReqMsg.setContentType(ctype);
}

int HttpReq::sendContentFile(const char* path) {
	return mMsgTx.sendContentFile(path);
}
} /* namespace cahttp */
