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
	FB_FIN = 0,
	FB_TE, // 1==Transfer-Encoding
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

ReUrlCtrl::ReUrlCtrl() : mCnnIf(this) {
	mpReqMsg = nullptr;
	mCnn = nullptr;
	mSendHandle = 0;
	mStatusFlag = 0;
	mSendDataCnt = 0;
	mContentLen = 0;
	mpServCnn = nullptr;

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
}

void ReUrlCtrl::OnHttpEnd() {
	ald("on end...");
}

int ReUrlCtrl::send(const char* ptr, size_t len) {
	auto wret = mCnn->send(mSendHandle, ptr, len);
	return wret;
}

void ReUrlCtrl::init(upBaseMsg upmsg, ReSvrCnn& cnn, uint32_t hsend) {
	mpReqMsg = upmsg.release();
	mpServCnn = &cnn;
	mCnn = mpServCnn->getConnection();
	mSendHandle = hsend;
//	mEndEvent.setOnListener([this](edft::EdEventFd &efd, int cnt) {
//		efd.close();
//		OnEnd();
//		mpServCnn->urlDummy(mHandle);
//	});
}

int ReUrlCtrl::response(int status_code, const char *pdata, size_t data_len,
		const char* ctype) {
	mSendDataCnt = 0;
	mRespMsg.setRespStatus(status_code);

//	if (F_TE()) {
//		mRespMsg.setTransferEncoding();
//	}
	mRespMsg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
	if(!F_GET(FB_CE)) {
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

	if(!F_TE() && mContentLen >= 0) {
		mRespMsg.setContentLen(mContentLen);
	}
	string msgstr = mRespMsg.serialize();
//		mCnn->setRecvIf(&mRecvIf);
//		mCnnHandle = mCnn->startSend(&mCnnIf);
	sendHttpMsg(move(msgstr));
	return 0;

}

int ReUrlCtrl::sendHttpMsg(std::string&& msg) {
	assert(msg.size() > 0);
//	ald("sending http msg: %s", msg);
	auto ret = mCnn->send(mSendHandle, msg.data(), msg.size());
	if (ret > 0) {
		// send fail
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
	int ret;
	for (; mBufList.empty() == false;) {
		auto *pktbuf = mBufList.front().get();
		auto buf = pktbuf->getBuf();
		alv("get buf, size=%ld", buf.first);
		if (buf.first > 0) {
			if (F_TE() && pktbuf->getType() == 0) {
				// writing chunk head
				char tmp[20];
				auto n = sprintf(tmp, "%lx\r\n", (size_t) buf.first);
				ret = mCnn->send(mSendHandle, tmp, n);
				if (ret > 0) {
					ald("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true);
					pktbuf->consume();
					goto END_SEND;
					break;
				}
			}
			ret = mCnn->send(mSendHandle, buf.second, buf.first);
			if (ret <= 0) {
				if (F_TE() && pktbuf->getType() == 0) {
					// writing chunk tail
					ret = mCnn->send(mSendHandle, "\r\n", 2);
					if (ret > 0) {
						ald("*** fail writing chunk ending line");
						stackTeByteBuf(nullptr, 0, false, false, true);
						pktbuf->consume();
						goto END_SEND;
						break;
					}
				}
				pktbuf->consume();
				if (ret < 0) {
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
		if(F_GET(FB_CE)) {
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
void ReUrlCtrl::OnHttpReqData(std::string&& data) {
}

int ReUrlCtrl::cnnif::OnWritable() {
	mpCtrl->procOnWritable();
	return 0;
}

int ReUrlCtrl::cnnif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	return 0;
}

int ReUrlCtrl::cnnif::OnData(std::string&& data) {
}

int ReUrlCtrl::cnnif::OnCnn(int cnnstatus) {
}

} /* namespace cahttp */
