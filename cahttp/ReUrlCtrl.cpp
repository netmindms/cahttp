/*
 * ReUrlCtrl.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG
#include "BytePacketBuf.h"
#include "StringPacketBuf.h"

#include "ReUrlCtrl.h"
#include "CaHttpCommon.h"
#include "ReSvrCnn.h"
#include "ext/nmdutil/FileUtil.h"
#include "ext/nmdutil/etcutil.h"
#include "ext/nmdutil/strutil.h"
#include "flog.h"

enum {
	FB_FIN = 0, FB_TE, // 1==Transfer-Encoding
	FB_CE, // content end
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

ReUrlCtrl::ReUrlCtrl() {
	mpReqMsg = nullptr;
	mCnn = nullptr;
	mHandle = 0;
	mStatusFlag = 0;
	mSendDataCnt = 0;
	mContentLen = 0;
	mpServCnn = nullptr;
	mRecvDataCnt = 0;
}

ReUrlCtrl::~ReUrlCtrl() {
	if (mpReqMsg) {
		delete mpReqMsg;
	}
}

std::vector<std::string>& cahttp::ReUrlCtrl::getPathParams() {
	return mPathParams;
}

void ReUrlCtrl::OnHttpReqMsgHdr(BaseMsg& msg) {
	ald("on msg header");
}

void ReUrlCtrl::OnHttpReqMsg(BaseMsg& msg) {
	ald("on req msg");
}

void ReUrlCtrl::OnHttpEnd() {
	ald("on end...");
}

int ReUrlCtrl::send(const char* ptr, size_t len) {
	auto wret = mpServCnn->send(mHandle, ptr, len);
	return wret;
}

void ReUrlCtrl::init(upBaseMsg upmsg, ReSvrCnn& cnn, uint32_t hsend) {
	mpReqMsg = upmsg.release();
	mpServCnn = &cnn;
	mCnn = mpServCnn->getConnection();
	mHandle = hsend;
//	mEndEvent.setOnListener([this](edft::EdEventFd &efd, int cnt) {
//		efd.close();
//		OnEnd();
//		mpServCnn->urlDummy(mHandle);
//	});
}

int ReUrlCtrl::response(int status_code, const char *pdata, size_t data_len, const char* ctype) {
	mSendDataCnt = 0;
	mRespMsg.setRespStatus(status_code);

//	if (F_TE()) {
//		mRespMsg.setTransferEncoding();
//	}
	mRespMsg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
	if (!F_GET(FB_CE)) {
		if (pdata) {
			auto *pb = new BytePacketBuf;
			pb->setData(pdata, data_len);
			mBufList.emplace_back();
			mBufList.back().reset(pb);
			if (ctype)
				mRespMsg.setContentType(ctype);
			if (!F_TE()) {
				mContentLen = data_len;
				mRespMsg.setContentLen(data_len);
			}
			F_SET(FB_CE);
		}
	}

	if (!F_TE() && mContentLen >= 0) {
		mRespMsg.setContentLen(mContentLen);
	}
	string msgstr = mRespMsg.serialize();
//		mCnn->setRecvIf(&mRecvIf);
//		mCnnHandle = mCnn->startSend(&mCnnIf);
	sendHttpMsg(move(msgstr));
	return 0;

}


int ReUrlCtrl::response(BaseMsg& msg) {
	mSendDataCnt = 0;
	mContentLen = msg.getContentLenInt();
	auto s = msg.serialize();
	auto sret = mpServCnn->send(mHandle, s.data(), s.size());
	if(sret == SEND_RESULT::SEND_OK) {
		if(mContentLen==0) {
			FS_FIN();
			mpServCnn->endCtrl(mHandle);
		}
		return 0;
	} else	if (sret == SEND_RESULT::SEND_NEXT) {
		stackSendBuf(move(s));
		return 0;
	} else if (sret == SEND_RESULT::SEND_PENDING) {
		return 0;
	} else {
		ale("### response error");
		return 1;
	}

	return 0;
}

int ReUrlCtrl::response(int status_code) {
	BaseMsg msg;
	setBasicHeader(msg, status_code);
	msg.setContentLen(0);
	return response(msg);
}

int ReUrlCtrl::response(int status_code, const std::string& content, const std::string& ctype) {
	BaseMsg msg;
	setBasicHeader(msg, status_code);
	msg.setContentType(ctype);
	msg.setContentLen(content.size());
	auto r = response(msg);
	if (r) {
		return r;
	}
	return writeContent(content.data(), content.size());
}

int ReUrlCtrl::sendHttpMsg(std::string&& msg) {
	assert(msg.size() > 0);
//	ald("sending http msg: %s", msg);
	auto ret = mpServCnn->send(mHandle, msg.data(), msg.size());
	if (ret == SEND_RESULT::SEND_NEXT) {
		// send next
		auto *pkt = new StringPacketBuf;
		pkt->setType(1);
		pkt->setString(move(msg));
		mBufList.emplace_front();
		mBufList.front().reset(pkt);
		return -1;
	} else {
		mCnn->reserveWrite();
		return 0;
	}
}

int ReUrlCtrl::procOnWritable() {
	ali("on writable");
	SEND_RESULT ret;
	for (; mBufList.empty() == false;) {
		auto *pktbuf = mBufList.front().get();
		auto buf = pktbuf->getBuf();
		alv("get buf, size=%ld", buf.first);
		if (buf.first > 0) {
			if (F_TE() && pktbuf->getType() == 0) {
				// writing chunk head
				char tmp[20];
				auto n = sprintf(tmp, "%lx\r\n", (size_t) buf.first);
				ret = mpServCnn->send(mHandle, tmp, n);
				if (ret == SEND_RESULT::SEND_NEXT || ret == SEND_RESULT::SEND_FAIL) {
//					assert(ret == SEND_RESULT::SEND_FAIL);
					ald("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true);
					pktbuf->consume();
					goto END_SEND;
					break;
				}
			}
			ret = mpServCnn->send(mHandle, buf.second, buf.first);
			if (ret <= 0) {
				if (F_TE() && pktbuf->getType() == 0) {
					// writing chunk tail
					ret = mpServCnn->send(mHandle, "\r\n", 2);
					if (ret == SEND_RESULT::SEND_NEXT || ret == SEND_RESULT::SEND_FAIL) {
						assert(ret == SEND_RESULT::SEND_FAIL);
						ald("*** fail writing chunk ending line");
						stackTeByteBuf(nullptr, 0, false, false, true);
						pktbuf->consume();
						goto END_SEND;
						break;
					}
				}
				pktbuf->consume();
				if (ret == SEND_RESULT::SEND_PENDING) {
					goto END_SEND;
					break;
				}
			} else {
				ali("*** fail writing chunk body, ...");
				if (F_TE() && pktbuf->getType() == 0) {
					stackTeByteBuf(buf.second, buf.first, false, true, true);
					pktbuf->consume();
					usleep(3 * 1000 * 1000);
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
	if (mBufList.empty() == true) {
		if (F_GET(FB_CE)) {
			OnHttpEnd();
		} else {
			OnHttpSendBufReady();
		}
	}
	END_SEND: ;
	return 0;
}

void ReUrlCtrl::OnHttpSendBufReady() {
}


void ReUrlCtrl::stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail) {
	auto *bf = new BytePacketBuf;
	bf->setType(2);
	bf->allocBuf(len + 20 + 4);

	if (head) {
		char tmp[20];
		auto n = snprintf(tmp, 20, "%lx\r\n", len);
		bf->addData(tmp, n);
	}

	if (body) {
		bf->addData(ptr, len);
	}

	if (tail) {
		bf->addData("\r\n", 2);
	}

	mBufList.emplace_front();
	mBufList.front().reset(bf);
}
void ReUrlCtrl::OnHttpReqData(std::string&& data) {

}

int ReUrlCtrl::writeContent(const char* ptr, size_t len) {
	if (F_TE() && (mSendDataCnt + (int64_t)len > mContentLen)) {
		ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
		return 1;
	}

	if (mBufList.empty() == true) {
		auto sret = mpServCnn->send(mHandle, ptr, len);
		if (sret == SEND_RESULT::SEND_NEXT) {
			stackSendBuf(ptr, len);
		}

		if (sret != SEND_RESULT::SEND_FAIL) {
			mSendDataCnt += len;
			return 0;
		} else {
			return 1;
		}
	} else {
		stackSendBuf(ptr, len);
		return 0;
	}
}

void ReUrlCtrl::stackSendBuf(std::string&& s) {
	auto *pbuf = new StringPacketBuf;
	pbuf->setString(move(s));
	mBufList.emplace_back(pbuf);
}

void ReUrlCtrl::stackSendBuf(const char* ptr, size_t len) {
	auto *pbuf = new BytePacketBuf;
	pbuf->setData(ptr, len);
	mBufList.emplace_back(pbuf);
}

int ReUrlCtrl::procOnData(std::string&& data) {
	alv("proc on data, size=%ld", data.size());
	if (data.size() == 0) {
		ald("empty data, request message end.");
		OnHttpReqMsg(*mpReqMsg);
		return 1;
	}
//	mRecvDataCnt += data.size();
//	if (mRecvDataBuf.empty() == true) {
//		mRecvDataBuf = move(data);
//	} else {
//		mRecvDataBuf.append(data);
//	}
	OnHttpReqData(move(data));
	return 0;
}

void ReUrlCtrl::setBasicHeader(BaseMsg& msg, int status_code) {
	msg.setMsgType(BaseMsg::MSG_TYPE_E::RESPONSE);
	msg.setRespStatus(status_code);
	msg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
}

bool ReUrlCtrl::isComplete() {
	return F_FIN();
}

} /* namespace cahttp */
