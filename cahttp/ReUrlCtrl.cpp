/*
 * ReUrlCtrl.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include "ReUrlCtrl.h"
#include "flog.h"


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

ReUrlCtrl::ReUrlCtrl() : mCnnIf(this) {
	mpReqMsg = nullptr;
	mCnn = nullptr;
	mSendHandle = 0;
	mStatusFlag = 0;
}

ReUrlCtrl::~ReUrlCtrl() {
	if(mpReqMsg) {
		delete mpReqMsg;
	}
}


std::vector<std::string>& cahttp::ReUrlCtrl::getPathParams() {
	return mPathParams;
}


void ReUrlCtrl::OnMsgHdr(BaseMsg& msg) {
	ald("on msg header");
}

void ReUrlCtrl::OnMsg(BaseMsg& msg) {
}

void ReUrlCtrl::OnEnd() {
	ald("on end...");
}

int ReUrlCtrl::send(const char* ptr, size_t len) {
	auto wret = mCnn->send(mSendHandle, ptr, len);
	return wret;
}

void ReUrlCtrl::init(upBaseMsg upmsg, BaseConnection& cnn, uint32_t hsend) {
	mpReqMsg = upmsg.release();
	mCnn = &cnn;
	mSendHandle = hsend;
}

int ReUrlCtrl::response(int status_code, const char *pdata, size_t data_len, const char* ctype) {
	mSendDataCnt = 0;
	mRespMsg.setRespStatus(status_code);

		if(F_TE()) {
			mRespMsg.setTransferEncoding();
		}
		mRespMsg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
		if(pdata) {
			if(ctype) mRespMsg.setContentType(ctype);
			if(!F_TE()) {
				mRespMsg.setContentLen(data_len);
			}
		} else {
			if(!F_TE() && mReqContentLen>=0) {
				mRespMsg.setContentLen(mReqContentLen);
			}
		}
		string msgstr = mRespMsg.serialize();
//		mCnn->setRecvIf(&mRecvIf);
//		mCnnHandle = mCnn->startSend(&mCnnIf);
		if(pdata) {
			auto *pb = new BytePacketBuf;
			pb->setData(pdata, data_len);
			mBufList.emplace_back();
			mBufList.back().reset(pb);
		}
		sendHttpMsg(move(msgstr));
		return 0;

}

void ReUrlCtrl::OnData(std::string&& data) {
}


int ReUrlCtrl::cnnif::OnWritable() {
}

int ReUrlCtrl::cnnif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
}

int ReUrlCtrl::cnnif::OnData(std::string&& data) {
}

int ReUrlCtrl::cnnif::OnCnn(int cnnstatus) {
}

} /* namespace cahttp */
