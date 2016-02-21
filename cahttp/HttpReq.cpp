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
	mStatus.val = 0;
	mErr = ERR::E_NO;
}

HttpReq::~HttpReq() {
}

int HttpReq::request(BaseMsg &msg) {
	mStatus.used = 1;
	mSendDataCnt = 0;
	uint32_t s_ip=0;
	int s_port=80;

	auto& url = msg.getUrl();
	if(mpCnn==nullptr) {
		mpCnn = new BaseConnection;
		mPropCnn.reset(mpCnn);
	}
	if (mSvrIp == 0) {
		CaHttpUrlParser parser;
		parser.parse(url);
		if(parser.port != "") {
			s_port = stoi(parser.port);
		}
		ali("resolving server ip, host=%s", parser.hostName);
		s_ip = get_ip_from_hostname(parser.hostName);
		ali("  ip=%s, port=%d", get_ipstr(mSvrIp), mSvrPort);
	}

	auto ret = mpCnn->connect(s_ip, s_port, 30000);
	ald("connecting, ret=%d", ret);

	if (mpCnn) {
		mRxHandle = mpCnn->openRxCh([this](BaseConnection::CH_E evt) ->int {
			ald("rx ch event=%d", (int)evt);
			if(evt == BaseConnection::CH_E::CH_MSG) {
				return procOnMsg();
			} else if(evt == BaseConnection::CH_E::CH_DATA) {
				return procOnData();
			} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
				if(!mStatus.fin) {
					ali("*** request early terminated");

					mErr = ERR::E_EARLY_DISCONNECTED;
					mStatus.fin = 1;
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
		if(mRxHandle==0) {
			return -1;
		}
		mTxHandle = mpCnn->openTxCh([this](BaseConnection::CH_E evt) ->int {
			ald("tx ch event=%d", (int)evt);
			if(evt == BaseConnection::CH_E::CH_WRITABLE) {
				return procOnWritable();
			} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
				ali("disconnected,...");
				if(!mStatus.fin) {
					ali("*** request early terminated");
					mErr = ERR::E_EARLY_DISCONNECTED;
					mStatus.fin = 1;
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
			mStatus.te = msg.getTransferEncoding();
			if(!mStatus.te) {
				mContentLen = msg.getContentLenInt();
			}
			bool content=false;
			if(mPresetContent.second) {
				msg.setContentType(mPresetContent.first);
				if(!mStatus.te) {
					msg.setContentLen(mPresetContent.second->remain());
				}
				content = true;
			}
			string msgstr = msg.serialize();
			auto r = sendHttpMsg(move(msgstr));
			if(!r && content) {
				if(mStatus.te) {
					mPresetContent.second->setType(1);
				}
				mBufList.emplace_back(mPresetContent.second.release());
				if(mStatus.te) {
					endData();
				}
			}
			return r;
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


int HttpReq::procOnWritable() {
	ald("proc writable, buf list cnt=%d", mBufList.size());
	int ret;
	for (; mBufList.empty() == false;) {
		auto *pktbuf = mBufList.front().get();
		auto buf = pktbuf->getBuf();
		alv("get buf, size=%ld", buf.first);
		if (buf.first > 0) {
			if(pktbuf->getType()) {
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
			if (ret == SEND_RESULT::SEND_OK || ret == SEND_RESULT::SEND_PENDING) {
				if(pktbuf->getType()) {
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
				if(pktbuf->getType()) {
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
		if(mStatus.se) {
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
	if(mpCnn) {
		if(mTxHandle) {
			closeTxCh();
		}
		if(mRxHandle) {
			closeRxCh();
		}
		mpCnn = nullptr;
		mSvrIp = 0;
		mSvrPort = 0;
		clear();
	}
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
	if(!mStatus.te && mContentLen == 0) {
		mStatus.se = 1;
	}
//	ald("sending http msg: %s", msg);
	auto ret = mpCnn->send(mTxHandle, msg.data(), msg.size());
	if (ret == SEND_NEXT || ret == SEND_FAIL) {
		// send fail
		stackSendBuf(move(msg));
	}
	if(ret == SEND_RESULT::SEND_OK || ret == SEND_RESULT::SEND_PENDING) {
		if(mStatus.se) {
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
		mStatus.fin = 1;
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
		if(!mStatus.fin) {
			ali("*** request terminated prematurely");
			mStatus.fin = 1;
			mLis(ON_END);
			return 1;
		}
	}
	return 0;
}

void HttpReq::transferEncoding(bool te) {
//	mStatus.te = te;
	mReqMsg.setTransferEncoding(te);
}

void HttpReq::endData() {
	if(mStatus.te && !mStatus.se) {
		if(mBufList.empty()==true) {
			auto ret = mpCnn->send(mTxHandle, "0\r\n\r\n", 5);
			if(ret==SEND_RESULT::SEND_OK || ret == SEND_RESULT::SEND_PENDING) {
				return;
			}
		} else {
			auto* pf = new TEEndPacketBuf;
			mBufList.emplace_back();
			auto &f = mBufList.back();
			f.reset(pf);
		}

	}
}

void HttpReq::stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail) {
	auto *bf = new BytePacketBuf;
	bf->setType(0);
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
		if(mStatus.te) {
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




void HttpReq::stackSendBuf(std::string&& s, bool te) {
	auto *pbuf = new StringPacketBuf;
	if(te) pbuf->setType(te);
	pbuf->setString(move(s));
	mBufList.emplace_back(pbuf);
}

void HttpReq::stackSendBuf(const char* ptr, size_t len, bool te) {
	auto *pbuf = new BytePacketBuf;
	if(te) pbuf->setType(te);
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
	// remove preset content
	if(mPresetContent.second) {
		mPresetContent.second.release();
	}

	setBasicHeader(mReqMsg, method);
	if(data_len>=0 && mReqMsg.getTransferEncoding()==false) {
		mReqMsg.setContentLen(data_len);
	}
	if(ctype) {
		mReqMsg.setContentType(ctype);
	}

	request(mReqMsg);
	if(pdata && data_len>0) {
		r=writeData(pdata, data_len);
	}

	return r;
}

int HttpReq::writeFile(const char* path) {
	if(mStatus.se) {
		return 1;
	}
	auto *pbuf = new FilePacketBuf;
	auto f = pbuf->open(path);
	if(!f) {
		auto len = pbuf->remain();
		if (!mStatus.te && (mSendDataCnt + (int64_t)len > mContentLen)) {
			ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
			goto err_exit;
		}
		mSendDataCnt += len;
		pbuf->setType(mStatus.te);
		mBufList.emplace_back(pbuf);
		mpCnn->reserveWrite();
		return 0;
	}
err_exit:
	delete pbuf;
	return 1;
}

int HttpReq::writeData(const char* ptr, size_t len) {
	if(mStatus.se) {
		return 1;
	}
	int ret=1;
	if (!mStatus.te && (mSendDataCnt + (int64_t)len > mContentLen)) {
		ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
		return 1;
	}

	mSendDataCnt += len;
	if(!mStatus.te && mSendDataCnt >= mContentLen) {
		mStatus.se=1;
		assert(mSendDataCnt==mContentLen);
	}
#if 0
	auto* pbuf = new BytePacketBuf;
	pbuf->setData(ptr, len);
	pbuf->setType(mStatus.te);
	mBufList.emplace_back(pbuf);
	mpCnn->reserveWrite();
	return 0;
#else
	if(mBufList.empty()) {
		SEND_RESULT sret;
		if(!mStatus.te) {
			sret = mpCnn->send(mTxHandle, ptr, len);
		} else {
			// writing chunk head
			char tmp[20];
			auto n = sprintf(tmp, "%lx\r\n", (size_t) len);
			sret = mpCnn->send(mTxHandle, tmp, n);
			if(sret == SEND_RESULT::SEND_OK || sret == SEND_RESULT::SEND_PENDING) {
				// write body
				sret = mpCnn->send(mTxHandle, ptr, len);
				if(sret == SEND_RESULT::SEND_OK || sret == SEND_RESULT::SEND_PENDING) {
					// write tail;
					sret = mpCnn->send(mTxHandle, "\r\n", 2);
					if(sret != SEND_RESULT::SEND_OK && sret != SEND_RESULT::SEND_PENDING) {
						stackTeByteBuf(ptr, len, false, false, true);
					}
				} else {
					stackTeByteBuf(ptr, len, false, true, true);
				}
			} else {
				stackTeByteBuf(ptr, len, true, true, true);
			}
		}

		if (sret == SEND_RESULT::SEND_OK ) {
			if(!mStatus.te && mStatus.se) {
				mpCnn->endTxCh(mTxHandle); mTxHandle=0;
			}
			ret = 0;
		} else if (sret == SEND_RESULT::SEND_NEXT || sret == SEND_RESULT::SEND_FAIL) {
			ret = -1;
		}
	} else {
		stackSendBuf(ptr, len, mStatus.te);
		ret = 0;
	}
	return ret;
#endif
}

void HttpReq::setContent(const char* ptr, size_t len, const std::string& ctype) {
	auto *pbuf = new BytePacketBuf;
	pbuf->setData(ptr, len);
	mPresetContent.first = ctype;
	mPresetContent.second.reset(pbuf);
}

int HttpReq::setContentFile(const char* path, const std::string& ctype) {
	auto *pbuf = new FilePacketBuf;
	auto r = pbuf->open(path);
	if(!r) {
		mPresetContent.first = ctype;
		mPresetContent.second.reset(pbuf);
	}
	return r;
}

void HttpReq::setContentLen(int64_t longInt) {
	mReqMsg.setTransferEncoding(false);
	mReqMsg.setContentLen(longInt);
}

int HttpReq::clear() {
	if(mStatus.used) {
		if(mStatus.fin==1) {
			assert(mTxHandle==0);
			assert(mRxHandle==0);
			mStatus.val = 0;
			mReqMsg.clear();
			mupRespMsg.reset();
			mRecvDataBuf.clear();
			mSendDataCnt=0;
			mRecvDataCnt=0;
			mContentLen=0;
			mPresetContent.first="";
			mPresetContent.second.reset();
			mErr = ERR::E_NO;
			mBufList.clear();
			return 0;
		} else {
			ale("### clearing not available in request active state");
			return -1;
		}
	} else {
		return 0;
	}
}

int HttpReq::txContent(const char* ptr, size_t len) {
	int ret=1;
	if (!mStatus.te && (mSendDataCnt + (int64_t)len > mContentLen)) {
		ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
		return 1;
	}

	mSendDataCnt += mContentLen;
	if(!mStatus.te && mSendDataCnt >= mContentLen) {
		mStatus.se=1;
	}

	if (mBufList.empty() == true) {
		auto sret = mpCnn->send(mTxHandle, ptr, len);
		if (sret == SEND_RESULT::SEND_OK || sret == SEND_RESULT::SEND_PENDING) {
			if(!mStatus.te && mStatus.se) {
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
	mReqMsg.setUrl(url);
	setBasicHeader(mReqMsg, method);
	if(!mStatus.te) {
		mReqMsg.setContentLen(data.size());
	}
	mReqMsg.setContentType(ctype);
	auto r = request(mReqMsg);
	if(!r) {
		r = writeData(data.data(), data.size());
	}
	if(mStatus.te) {
		endData();
	}
	mStatus.se = 1;
	return r;
}

} /* namespace cahttp */
