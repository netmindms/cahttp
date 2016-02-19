/*
 * HttpReq.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE
#include "flog.h"

#include "HttpReq.h"
#include "FilePacketBuf.h"
#include "CaHttpUrlParser.h"
#include "StringPacketBuf.h"
#include "CaHttpCommon.h"
#include "BytePacketBuf.h"
#include "TEEndPacketBuf.h"

#include "ext/nmdutil/FileUtil.h"
#include "ext/nmdutil/etcutil.h"
#include "ext/nmdutil/strutil.h"

enum {
	FB_FIN=0,
	FB_TE, // 1==Transfer-Encoding
	FB_SE, // sending end
};

#define F_GET(A) BIT_TEST(mStatusFlag, A)
#define F_SET(A) BIT_SET(mStatusFlag, A)
#define F_RESET(A) BIT_RESET(mStatusFlag, A)

#define F_FIN() F_GET(FB_FIN)
#define FS_FIN() F_SET(FB_FIN)
#define FR_FIN() F_RESET(FB_FIN)

#define F_TE() F_GET(FB_TE)
#define FS_TE() F_SET(FB_TE)
#define FR_TE() F_RESET(FB_TE)

using namespace std;
using namespace cahttpu;

namespace cahttp {

HttpReq::HttpReq() {
	mSvrIp = 0;
	mSvrPort = 80;
	mpCnn = nullptr;
	mRecvDataCnt = 0;
	mContentLen = 0;
	mSendDataCnt = 0;
	mRxHandle = 0;
	mTxHandle = 0;
	mStatusFlag = 0;
}

HttpReq::~HttpReq() {
}

int HttpReq::request(BaseMsg &msg) {
	mSendDataCnt = 0;
	auto& url = msg.getUrl();
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
		if(F_TE()) {
			msg.setTransferEncoding();
		} else {
			mContentLen = msg.getContentLenInt();
		}

		string msgstr = msg.serialize();
		mRxHandle = mpCnn->openRxCh([this](BaseConnection::CH_E evt) {
			ald("rx ch event=%d", (int)evt);
			if(evt == BaseConnection::CH_E::CH_MSG) {
				return procOnMsg();
			} else if(evt == BaseConnection::CH_E::CH_DATA) {
				return procOnData();
			} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
				if(F_FIN()==0) {
					ali("*** request terminated prematurely");
					FS_FIN();
					mLis(ON_END);
				}
				closeTxCh(); // close tx ch manually,
				return 0;
			} else {
				assert(0);
				return 1;
			}
		});
		ald("get rx ch=%d", mRxHandle);

		mTxHandle = mpCnn->openTxCh([this](BaseConnection::CH_E evt) {
			ald("tx ch event=%d", (int)evt);
			if(evt == BaseConnection::CH_E::CH_WRITABLE) {
				return procOnWritable();
			} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
				ali("disconnected,...");
				if(F_FIN()==0) {
					ali("*** request terminated prematurely");
					FS_FIN();
					mLis(ON_END);
				}
				closeRxCh(); // close rx ch manually
				return 0;
			} else {
				assert(0);
				return 1;
			}
		});
		ald("get tx ch=%d", mTxHandle);

		if (mTxHandle) {
			sendHttpMsg(move(msgstr));
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

int HttpReq::sendPacket(const char* buf, size_t len) {
	auto ret = mpCnn->send(mTxHandle, buf, len);
	if (ret > 0) {
		// send fail
		auto *pkt = new StringPacketBuf;
		pkt->setString(buf, len);
		mBufList.emplace_back();
		mBufList.back().reset(pkt);
		return -1;
	} else {
		return ret;
	}
}

int HttpReq::sendPacket(string&& s) {
	auto ret = mpCnn->send(mTxHandle, s.data(), s.size());
	if (ret > 0) {
		// send fail
		auto *pkt = new StringPacketBuf;
		pkt->setString(move(s));
		mBufList.emplace_back();
		mBufList.back().reset(pkt);
		return -1;
	} else {
		return ret;
	}
}

int HttpReq::request_get(const std::string& url, Lis lis) {
	mLis = lis;
	BaseMsg msg;
	msg.setUrl(url);
	setBasicHeader(msg, HTTP_GET);
	msg.setContentLen(0);
	return request(msg);
}


int HttpReq::request_post(const std::string& url, Lis lis) {
	mLis = lis;
	BaseMsg msg;
	msg.setUrl(url);
	setBasicHeader(msg, HTTP_POST);
	msg.setContentLen(0);
	return request(msg);
}


int HttpReq::procOnWritable() {
	ald("proc writable, buf list cnt=%d", mBufList.size());
	int ret;
	for (; mBufList.empty() == false;) {
		auto *pktbuf = mBufList.front().get();
		auto buf = pktbuf->getBuf();
		alv("get buf, size=%ld", buf.first);
		if (buf.first > 0) {
			if(F_TE() && pktbuf->getType()==0) {
				// writing chunk head
				char tmp[20];
				auto n = sprintf(tmp, "%lx\r\n", (size_t)buf.first);
				ret = mpCnn->send(mTxHandle, tmp, n);
				if(ret > 0 ) {
					ald("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true);
					pktbuf->consume();
					goto END_SEND;
					break;
				}
			}
			ret = mpCnn->send(mTxHandle, buf.second, buf.first);
			if (ret <= 0) {
				if(F_TE() && pktbuf->getType()==0) {
					// writing chunk tail
					ret = mpCnn->send(mTxHandle, "\r\n", 2);
					if(ret>0) {
						ald("*** fail writing chunk ending line");
						stackTeByteBuf(nullptr, 0, false, false, true);
						pktbuf->consume();
						goto END_SEND;
						break;
					}
				}
				pktbuf->consume();
				if(ret<0) {
					goto END_SEND;
					break;
				}
			} else {
				ali("*** fail writing chunk body, ...");
				if(F_TE() && pktbuf->getType()==0) {
					stackTeByteBuf(buf.second, buf.first, false, true, true);
					pktbuf->consume();
					usleep(3*1000*1000);
					goto END_SEND;
					break;
				}
				goto END_SEND;
				break;
			}
		} else {
			mBufList.pop_front();
		}
	}
	alv("  buf list count=%d", mBufList.size());
	if(mBufList.empty()==true) {
		if(F_GET(FB_SE)) {
			ald("sending ended.");
			mpCnn->endTxCh(mTxHandle); mTxHandle=0;
		}
		mLis(ON_SEND);
	} else {

	}
END_SEND:
	;
	return 0;
}

//int HttpReq::ReqCnnIf::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
//	return mpReq->procOnMsg(move(upmsg));
//}

int HttpReq::getRespStatus() {
	if(mupRespMsg) {
		return mupRespMsg->getRespStatus();
	} else {
		ald("*** no response message");
		return 0;
	}
}

int64_t HttpReq::getRespContentLen() {
	return mupRespMsg->getContentLenInt();
}

int HttpReq::procOnMsg() {
	mupRespMsg.reset( mpCnn->fetchMsg() );
	assert(mLis);
	mLis(ON_MSG);

	return 0;
//	if(mupRespMsg->getRespStatus() >= 200) {
//		ali("final response message, status=%d", mupRespMsg->getRespStatus());
//		if(mupRespMsg->getContentLenInt()==0) {
//			FS_FIN();
//			mLis(ON_END);
//		}
//	}
}

//int HttpReq::ReqCnnIf::OnData(std::string&& data) {
//	return mpReq->procOnData(data);
//}


void HttpReq::close() {
	if(mPropCnn) {
		mPropCnn->close();
		mPropCnn.reset();
	}
}

void HttpReq::setReqContent(const std::string& data, const std::string& content_type) {
	if(data.size()>0) {
		mContentLen = data.size();
		mReqMsg.setContentType(content_type);
		StringPacketBuf *pbuf = new StringPacketBuf;
		pbuf->setString(data.data(), data.size());
		mBufList.emplace_back();
		mBufList.back().reset(pbuf);
	}
}

int HttpReq::setReqContentFile(const std::string& path, const std::string& content_type) {
	auto *pbuf = new FilePacketBuf;
	auto ret = pbuf->open(path);
	if(!ret) {
		ald("set content file, size=%lu", pbuf->remain());
		mBufList.emplace_back();
		mBufList.back().reset(pbuf);
		mContentLen = cahttpu::FileUtil::getSize(path);
		mReqMsg.setContentType(content_type);
	} else {
		delete pbuf;
	}
	return ret;
}


int HttpReq::sendHttpMsg(std::string&& msg) {
	assert(msg.size()>0);
	if(!F_TE() && mContentLen <= mSendDataCnt) {
		F_SET(FB_SE);
	}
//	ald("sending http msg: %s", msg);
	auto ret = mpCnn->send(mTxHandle, msg.data(), msg.size());
	if (ret == SEND_NEXT || ret == SEND_FAIL) {
		// send fail
		stackSendBuf(move(msg));
	}
	if(ret == SEND_RESULT::SEND_OK || ret == SEND_RESULT::SEND_PENDING) {
		if(F_GET(FB_SE)) {
			mpCnn->endRxCh(mTxHandle); mTxHandle=0;
		}
	}
	return 0;
}

std::string HttpReq::fetchData() {
	return move(mRecvDataBuf);
}

int HttpReq::procOnData() {
	auto data = mpCnn->fetchData();
	alv("proc on data, size=%ld", data.size());
	if(data.size()==0) {
		ald("empty data, consider as message end signal,");
		mpCnn->endRxCh(mRxHandle); mRxHandle=0;
		FS_FIN();
		mLis(ON_END);
		return 1;
	}
	mRecvDataCnt += data.size();
	if(mRecvDataBuf.empty()==true) {
		mRecvDataBuf = move(data);
	} else {
		mRecvDataBuf.append(data);
	}
	mLis(ON_DATA);
	return 0;
//	if(mRecvDataCnt == mupRespMsg->getContentLenInt()) {
//		FS_FIN();
//		mLis(ON_END);
//	}
}

int HttpReq::procOnCnn(int status) {
	if(status==0) {
		ali("disconnected,...");
		if(F_FIN()==0) {
			ali("*** request terminated prematurely");
			FS_FIN();
			mLis(ON_END);
			return 1;
		}
	}
	return 0;
}

void HttpReq::transferEncoding(bool te) {
	if(te) FS_TE();
	else FR_TE();
}

void HttpReq::endData() {
	if(F_TE()) {
		if(mBufList.empty()==true) {
			auto ret = mpCnn->send(mTxHandle, "0\r\n\r\n", 5);
			if(ret<=0) {
				return;
			}
		}

		auto* pf = new TEEndPacketBuf;
		mBufList.emplace_back();
		auto &f = mBufList.back();
		f.reset(pf);
	}
}

void HttpReq::stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail) {
	auto *bf = new BytePacketBuf;
	bf->setType(2);
	bf->allocBuf(len+20+4);

	if(head) {
		char tmp[20];
		auto n = snprintf(tmp, 20, "%lx\r\n", len);
		bf->addData(tmp, n);
	}

	if(body) {
		bf->addData(ptr, len);
	}

	if(tail) {
		bf->addData("\r\n", 2);
	}

	mBufList.emplace_front();
	mBufList.front().reset(bf);
}

int HttpReq::sendData(const char* ptr, size_t len) {
	if(mpCnn->isWritable()==true && mBufList.empty()==true) {
		if(F_TE()) {
			string s;
			s.reserve(len+20);
			s = cahttpu::fmt::format("{:x}\r\n", len);
			s.append(ptr, len);
			s.append("\r\n");
			auto wret = mpCnn->send(mTxHandle, s.data(), s.size());
			return wret;
		} else {
			auto wret = mpCnn->send(mTxHandle, ptr, len);
			return wret;
		}
	} else {
		return 1;
	}
}

#if 0
HttpReq::localrecvif::localrecvif(HttpReq& r): mReq(r) {
}

HttpReq::localrecvif::~localrecvif() {
}

int HttpReq::localrecvif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	return mReq.procOnMsg(move(upmsg));
}

int HttpReq::localrecvif::OnData(std::string&& data) {
	return mReq.procOnData(data);
}
#endif




void HttpReq::stackSendBuf(std::string&& s) {
	auto *pbuf = new StringPacketBuf;
	pbuf->setString(move(s));
	mBufList.emplace_back(pbuf);
}

void HttpReq::stackSendBuf(const char* ptr, size_t len) {
	auto *pbuf = new BytePacketBuf;
	pbuf->setData(ptr, len);
	mBufList.emplace_back(pbuf);
}

void HttpReq::setBasicHeader(BaseMsg& msg, http_method method) {
	msg.setMsgType(BaseMsg::MSG_TYPE_E::REQUEST);
	msg.setMethod(method);
	msg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
}

int HttpReq::request(http_method method, const char* pdata, size_t data_len, const char* ctype) {
	int r=0;
	BaseMsg msg;
	setBasicHeader(msg, method);
	if(data_len>=0) {
		msg.setContentLen(data_len);
	}
	if(ctype) {
		msg.setContentType(ctype);
	}

	request(msg);
	if(pdata && data_len>0) {
		r=writeContent(pdata, data_len);
	}
	return r;
}

int HttpReq::writeContentFile(const char* path) {
	if(F_GET(FB_SE)) {
		return 1;
	}
	auto *pbuf = new FilePacketBuf;
	auto f = pbuf->open(path);
	if(!f) {
		auto len = pbuf->remain();
		if (!F_TE() && (mSendDataCnt + (int64_t)len > mContentLen)) {
			ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
			goto err_exit;
		}
		mSendDataCnt += len;
		mBufList.emplace_back(pbuf);
		mpCnn->reserveWrite();
		return 0;
	}
err_exit:
	delete pbuf;
	return 1;
}

int HttpReq::writeContent(const char* ptr, size_t len) {
	if(F_GET(FB_SE)) {
		return 1;
	}
	auto r = txContent(ptr, len);
	if(r<0) {
		stackSendBuf(ptr, len);
	}
	return r;
}

int HttpReq::txContent(const char* ptr, size_t len) {
	int ret=1;
	if (!F_TE() && (mSendDataCnt + (int64_t)len > mContentLen)) {
		ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
		return 1;
	}

	mSendDataCnt += mContentLen;
	if(!F_TE() && mSendDataCnt >= mContentLen) {
		F_SET(FB_SE);
	}

	if (mBufList.empty() == true) {
		auto sret = mpCnn->send(mTxHandle, ptr, len);
		if (sret == SEND_RESULT::SEND_OK || sret == SEND_RESULT::SEND_PENDING) {
			if(!F_TE() && F_GET(FB_SE)) {
				mpCnn->endTxCh(mTxHandle); mTxHandle=0;
			}
			ret = 0;
		} else if (sret == SEND_RESULT::SEND_NEXT) {
			ret = -1;
		}
	} else {
		ret = -1;
	}
	return ret;
}



int HttpReq::request(http_method method, const std::string& url, const std::string& data, const std::string& ctype, Lis lis) {
	mLis = lis;
	BaseMsg msg;
	msg.setUrl(url);
	setBasicHeader(msg, method);
	msg.setContentLen(data.size());
	msg.setContentType(ctype);
	request(msg);
	writeContent(data.data(), data.size());
	return 0;
}

} /* namespace cahttp */
