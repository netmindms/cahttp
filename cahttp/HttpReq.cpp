/*
 * HttpReq.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG
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

HttpReq::HttpReq() : mCnnIf(this) {
	mSvrIp = 0;
	mSvrPort = 80;
	mpCnn = nullptr;
	mCnnHandle = 0;
	mRecvDataCnt = 0;
	mReqContentLen = 0;
	mStatusFlag = 0;
	mSendDataCnt = 0;
}

HttpReq::~HttpReq() {
}

int HttpReq::request(http_method method, const char *pdata, size_t data_len, const char* ctype) {
	mSendDataCnt = 0;
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
		if(F_TE()) {
			mReqMsg.setTransferEncoding();
		}
		mReqMsg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
		if(pdata) {
			if(ctype) mReqMsg.setContentType(ctype);
			if(!F_TE()) {
				mReqMsg.setContentLen(data_len);
			}
		} else {
			if(!F_TE() && mReqContentLen>=0) {
				mReqMsg.setContentLen(mReqContentLen);
			}
		}
		string msgstr = mReqMsg.serialize();
		mCnnHandle = mpCnn->startSend(&mCnnIf);
		if (mCnnHandle) {
			if(pdata) {
				auto *pb = new BytePacketBuf;
				pb->setData(pdata, data_len);
				mBufList.emplace_back();
				mBufList.back().reset(pb);
			}
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

HttpReq::ReqCnnIf::ReqCnnIf(HttpReq* req) {
	mpReq = req;
}

HttpReq::ReqCnnIf::~ReqCnnIf() {
}

int HttpReq::ReqCnnIf::OnWritable() {
	return mpReq->procWritable();
}

int HttpReq::sendPacket(const char* buf, size_t len) {
	auto ret = mpCnn->send(mCnnHandle, buf, len);
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
	auto ret = mpCnn->send(mCnnHandle, s.data(), s.size());
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
	mReqMsg.setUrl(url);
	return request(HTTP_GET);
}

#if 0
void HttpReq::procWritable() {
//	ald("proc writable, buf list cnt=%d", mBufList.size());
	int ret;
	for (; mBufList.empty() == false;) {
		auto *pktbuf = mBufList.front().get();
		auto buf = pktbuf->getBuf();
//		ald("get buf, size=%ld", buf.first);
		if (buf.first > 0) {
			if(F_TE() && pktbuf->getType()==0) {
				string sbuf;
				sbuf = cahttp::fmt::format("{:x}\r\n", buf.first);
				sbuf.append(buf.second, buf.first);
				sbuf.append("\r\n");
				ret = mpCnn->send(sbuf.data(), sbuf.size());
			} else {
				ret = mpCnn->send(buf.second, buf.first);
			}
			if (ret <= 0) {
				pktbuf->consume();
				if(ret<0) break;
			} else {
				break;
			}
		} else {
			mBufList.pop_front();
		}
	}
	ald("  buf list count=%d", mBufList.size());
}
#else
int HttpReq::procWritable() {
	alv("proc writable, buf list cnt=%d", mBufList.size());
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
				ret = mpCnn->send(mCnnHandle, tmp, n);
				if(ret > 0 ) {
					alv("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true);
					pktbuf->consume();
					goto END_SEND;
					break;
				}
			}
			ret = mpCnn->send(mCnnHandle, buf.second, buf.first);
			if (ret <= 0) {
				if(F_TE() && pktbuf->getType()==0) {
					// writing chunk tail
					ret = mpCnn->send(mCnnHandle, "\r\n", 2);
					if(ret>0) {
						alv("*** fail writing chunk ending line");
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
		mLis(ON_SEND);
	}
END_SEND:
	;
	return 0;
}
#endif

int HttpReq::ReqCnnIf::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	return mpReq->procOnMsg(move(upmsg));
}

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

int HttpReq::procOnMsg(std::unique_ptr<BaseMsg> upmsg) {
	mupRespMsg = move(upmsg);
	assert(mLis);
	mLis(ON_MSG);

//	if(mupRespMsg->getRespStatus() >= 200) {
//		ali("final response message, status=%d", mupRespMsg->getRespStatus());
//		if(mupRespMsg->getContentLenInt()==0) {
//			FS_FIN();
//			mLis(ON_END);
//		}
//	}
}

int HttpReq::ReqCnnIf::OnData(std::string&& data) {
	return mpReq->procOnData(data);
}

int HttpReq::ReqCnnIf::OnCnn(int cnnstatus) {
	return mpReq->procOnCnn(cnnstatus);
}

void HttpReq::close() {
	if(mPropCnn) {
		mPropCnn->close();
		mPropCnn.reset();
	}
}

void HttpReq::setReqContent(const std::string& data, const std::string& content_type) {
	if(data.size()>0) {
		mReqContentLen = data.size();
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
		mReqContentLen = cahttpu::FileUtil::getSize(path);
		mReqMsg.setContentType(content_type);
	} else {
		delete pbuf;
	}
	return ret;
}


int HttpReq::request_post(const std::string& url, Lis lis) {
	mLis = lis;
	mReqMsg.setUrl(url);
	return request(HTTP_POST);
}

int HttpReq::sendHttpMsg(std::string&& msg) {
	assert(msg.size()>0);
//	ald("sending http msg: %s", msg);
	auto ret = mpCnn->send(mCnnHandle, msg.data(), msg.size());
	if (ret > 0) {
		// send fail
		auto *pkt = new StringPacketBuf;
		pkt->setType(1);
		pkt->setString(move(msg));
		mBufList.emplace_front();
		mBufList.front().reset(pkt);
		return -1;
	} else {
		// TODO:
		if(mBufList.size()>0) {
			mpCnn->reserveWrite();
		}
		return ret;
	}
}

std::string HttpReq::fetchData() {
	return move(mRecvDataBuf);
}

int HttpReq::procOnData(std::string& data) {
	if(data.size()==0) {
		ald("empty data, consider as message end signal,");
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
			auto ret = mpCnn->send(mCnnHandle, "0\r\n\r\n", 5);
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
			auto wret = mpCnn->send(mCnnHandle, s.data(), s.size());
			return wret;
		} else {
			auto wret = mpCnn->send(mCnnHandle, ptr, len);
			return wret;
		}
	} else {
		return 1;
	}
}

} /* namespace cahttp */
