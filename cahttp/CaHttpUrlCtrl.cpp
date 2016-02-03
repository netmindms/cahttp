/*
 * CaHttpUrlCtrl.cpp
 *
 *  Created on: Apr 24, 2015
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include "flog.h"
#include "CaHttpUrlCtrl.h"
#include "ServCnn.h"
#include "http_parser.h"
#include "HttpStringReadStream.h"

using namespace std;

namespace cahttp {

#define CLEN_MASK 1
#define EOS_MASK 2
#define WM_MASK 4 // write mode mask, 0:stream, 1:direct
#define CTRL_SHUTDOWN 8

#define TRANSFER_ENC() (mFlag & CLEN_MASK)
#define SET_TRANSFER_ENC() (mFlag |= CLEN_MASK)
#define EOS() (mFlag & EOS_MASK)
#define SET_EOS() (mFlag |= EOS_MASK)
#define RESET_EOS() (mFlag &= (~EOS_MASK))
#define SET_SWMODE() (mFlag |= WM_MASK)
#define SET_DWMODE() (mFlag &= (~WM_MASK))
#define IS_SWMODE() (mFlag & WM_MASK)
#define SET_SHUTDOWN() (mFlag |= CTRL_SHUTDOWN)
#define IS_SHUTDOWN() (mFlag & CTRL_SHUTDOWN)

class DefReqDataStrm: public HttpBaseWriteStream {
public:
	DefReqDataStrm(size_t maxsize) {
		mMaxSize = maxsize;
	};
	ssize_t write(const char* ptr, size_t len) override {
		ali("strm size=%ld", size());
		if(mStrBuf.size()+len <= mMaxSize) {
			mStrBuf.append(ptr, len);
			return len;
		} else {
			return -1;
		}
	}
	const string& getString() {
		return mStrBuf;
	}
	size_t size() override {
		return mStrBuf.size();
	}
	void end() override {

	}
private:
	size_t mMaxSize;
	string mStrBuf;
};

CaHttpUrlCtrl::CaHttpUrlCtrl() {
	mCnn = nullptr;
	mRespContentLen = 0;
	mFlag = 0;
	mWriteCnt = 0;
	mRespCode = 0;
	mHandle = 0;
	mDataReadCnt = 0;
	mReqDataStrm = nullptr;
	mDataStrm = nullptr;
	ald("url ctrl const, ptr=%x", (long)this);
}

CaHttpUrlCtrl::~CaHttpUrlCtrl() {
	ald("url ctrl dest...ptr=%x", (uint64_t)this);
//	if(mpPendigStrm) {
//		delete mpPendigStrm;
//	}
}

void CaHttpUrlCtrl::OnHttpReqMsgHdr() {
}

void CaHttpUrlCtrl::OnHttpReqData(string &&data) {
}

void CaHttpUrlCtrl::OnHttpReqDataEnd() {
}

void CaHttpUrlCtrl::OnHttpReqMsg() {
}

void CaHttpUrlCtrl::OnHttpSendBufReady() {
}

void CaHttpUrlCtrl::OnHttpEnd() {
}

CaHttpSvrReq& CaHttpUrlCtrl::getReq() {
	return mReq;
}

void CaHttpUrlCtrl::response(int status, string&& data, string&& content_type) {
	mRespMsg.setHdr(CAS::HS_CONTENT_TYPE, content_type);
	mRespContentLen = data.size();
	auto &strm = *(new HttpStringReadStream);
	strm.setString(move(data));
	mpuSelfDataStrm.reset(&strm);
	mDataStrm = &strm;
	response(status);

}

void CaHttpUrlCtrl::response(int status, const std::string& data, const std::string& content_type) {
	response(status, string(data), string(content_type));
}


void CaHttpUrlCtrl::response(int status, HdrList && hdrs, HttpBaseReadStream *datastrm) {
	mRespMsg.setHdrList(move(hdrs));
	mDataStrm = datastrm;
	response(status);
}

void CaHttpUrlCtrl::response(int status) {
	mWriteCnt = 0;
	mRespCode = status;
	RESET_EOS();
	if(mDataStrm) {
		SET_SWMODE();
	} else {
		SET_DWMODE();
	}
	if(mRespContentLen == -1) {
		mRespMsg.setHdr("Transfer-Encoding", "chunked");
	} else {
		mRespMsg.setHdr(CAS::HS_CONTENT_LEN, to_string(mRespContentLen));
	}
	mRespMsg.setStatus(status);
	string ds;
	time_t t;
	time(&t);
	get_http_date_str(ds, &t);
	mRespMsg.setHdr(CAS::HS_DATE, move(ds));
	mHdrStrm.setString(mRespMsg.serialize());
	mCnn->transfer(this);
}

int CaHttpUrlCtrl::getRespCode() {
	return mRespCode;
}

void CaHttpUrlCtrl::setConnection(ServCnn& cnn) {
	mCnn = &cnn;
}

void CaHttpUrlCtrl::setUrlMatchResult(const smatch& match) {
	mUrlMatchRes.clear();
	for (size_t i = 1; i < match.size(); i++) {
		mUrlMatchRes.push_back(match[i]);
	}
}

string CaHttpUrlCtrl::dump() {
	string ds;
	ds += "* url match: ";
	for (auto &s : mUrlMatchRes) {
		ds += s + ", ";
	}
	ds += "\n";

	return move(ds);
}

const vector<string>& CaHttpUrlCtrl::getUrlMatchStr() {
	return mUrlMatchRes;
}

void CaHttpUrlCtrl::OnHttpReqNonHttpData(const char* ptr, size_t len) {
}



int CaHttpUrlCtrl::sendData(const char* ptr, size_t len) {
	ald("send data, len=%ld", len);
	assert(mHdrStrm.remain()==0);

	if(mWriteCnt+len > mRespContentLen) {
		assert(0);
		return -2;
	}
	if(EOS()) {
		alw("*** send blocked because of EOS");
		assert(0);
		return -1;
	}
	int wret;
	int64_t wcnt;
	if(!TRANSFER_ENC()) {
		auto remain = mRespContentLen - mWriteCnt;
		wcnt = min((int64_t)len, remain);
		wret = mCnn->writeData(*this, ptr, wcnt);
		ald("cnn write data, ret=%d", wret);
	} else {
		// TODO: size limit check
		wcnt = len;
		wret = writeTec(ptr, wcnt);
	}
	if(wret != SEND_FAIL) {
		mWriteCnt += wcnt;
		if(!TRANSFER_ENC()) {
			if(mWriteCnt == mRespContentLen) {
				SET_EOS();
			}
		} else {
			if(len == 0) {
				SET_EOS();
			}
		}

		if(wret == SEND_OK) {
			if(mWriteCnt == mRespContentLen) {
				mCnn->reserveWrite();
			}
		} else {
			// SEND_PENDING
			ald("send pending...");
		}
		return 0;
	} else {
		ald("*** send fail...");
		return -1;
	}
}


std::pair<const char*, int64_t> CaHttpUrlCtrl::getDataPtr() {
//	if(EOS()) {
//		return {nullptr, 0};
//	}
	if(mHdrStrm.remain()>0) {
		auto dp = mHdrStrm.getDataPtr();
		return dp;
	} else {
		if(!TRANSFER_ENC()) {
			if(mDataStrm) {
				return mDataStrm->getDataPtr();
			}
			return {nullptr, -1};
		} else {
			if(mChkStrm && mChkStrm->remain()>0) {
				return mChkStrm->getDataPtr();
			}
			if(mDataStrm) {
				if(EOS()) {
					return {nullptr, 0};
				}
				auto dp = mDataStrm->getDataPtr();
				if(dp.second == -1) {
					return {nullptr, -1};
				}
				size_t chksize = min((size_t)4096, (size_t)dp.second);
				string s;
				if(chksize>=0) {
					char hexbuf[30];
					snprintf(hexbuf, 20, "%lx\r\n", chksize);
					s = hexbuf;
				}
				if(chksize>0) {
					s.append(dp.first, chksize);
					mDataStrm->consume(chksize);
				}
				s += "\r\n";
				if(mDataStrm->remain()==0) {
					s += "0\r\n\r\n";
				}

				if(mChkStrm == nullptr) mChkStrm.reset(new HttpStringReadStream);
				mChkStrm->setString(move(s));
				return mChkStrm->getDataPtr();
			} else {
				return {nullptr, -1};
			}
		}
	}
}

void CaHttpUrlCtrl::consume(size_t len) {
	ald("consume, len=%d, te=%d, handle=%d", len, TRANSFER_ENC(), mHandle);
	if(mHdrStrm.remain()>0) {
		mHdrStrm.consume(len);
		if(mRespContentLen==0) {
			SET_EOS();
		}
	} else {
		if(!TRANSFER_ENC()) {
			mDataReadCnt += len;
			if(mDataStrm) mDataStrm->consume(len);
			if(mDataReadCnt == mRespContentLen) {
				SET_EOS();
			}
		} else {
			if(mChkStrm) {
				ald("    slice strm remain=%d", mChkStrm->remain());
				if(mChkStrm->remain()==5) {
					ali("test break...chk ptr=%lx", (long)mChkStrm.get());
				}
				assert(len == mChkStrm->remain());
				mChkStrm->consume(len);
			}

			if(mDataStrm && mDataStrm->remain()==0) {
				SET_EOS();
			}

		}
	}

}

void CaHttpUrlCtrl::consumeData(size_t len) {
	mWriteCnt += len;
	if(mDataStrm) {
		mDataStrm->consume(len);
	}
}


void CaHttpUrlCtrl::addRespHdr(const string& name, const string& val) {
	mRespMsg.setHdr(name, val);
}



bool CaHttpUrlCtrl::isStreamMode() {
	if(mDataStrm) {
		return true;
	} else {
		return false;
	}
}

bool CaHttpUrlCtrl::isSendComp() {
	return EOS();
}

void CaHttpUrlCtrl::setRespContent(const char* ptr, int64_t data_len, string&& ctype, bool tec) {
	if(ptr && data_len > 0) {
		mRespContentLen = data_len;
	} else {
		assert(0);
		return;
	}
	if(tec) {
		mRespContentLen = -1;
		SET_TRANSFER_ENC();
	}
	if(ctype.empty()==false) {
		addRespHdr(string(CAS::HS_CONTENT_TYPE), string(ctype));
	}
	mWriteCnt = 0;
	auto &strm = *new HttpStringReadStream;
	strm.setString(string(ptr, (size_t)data_len));
	mpuSelfDataStrm.reset(&strm);
	mDataStrm = &strm;
}

void CaHttpUrlCtrl::setRespContent(HttpBaseReadStream *strm, int64_t len) {
	mDataStrm = strm;
	mRespContentLen = len;
	if(len<0) {
		SET_TRANSFER_ENC();
	}
}

const string& CaHttpUrlCtrl::getReqHdr(const char* name) {
	return mReq.getReqMsg().getHdrOne(name);
}

uint32_t CaHttpUrlCtrl::getHandle() {
	return mHandle;
}

void CaHttpUrlCtrl::setHandle(uint32_t handle) {
	assert(mHandle==0);
	mHandle = handle;
}

uint64_t CaHttpUrlCtrl::getContentLen() {
	return mRespContentLen;
}


int CaHttpUrlCtrl::procDataLack() {
	ald("proc data lack,..., te=%d, eos=%d", TRANSFER_ENC(), EOS());
	if(EOS()) {
		ald("  EOS dectected, ...");
		return 0;
	} else {
		OnHttpSendBufReady();
		return 1; // TODO:
	}
}

//int CaHttpUrlCtrl::writePendingData(const char* ubf, size_t len) {
//	assert(mpPendigStrm);
//	// TODO: max size check
//	mpPendigStrm->put(ubf, len);
//	return 0;
//}
//
//RingBufReadStream* CaHttpUrlCtrl::getPendingStrm() {
//	return mpPendigStrm;
//}

void CaHttpUrlCtrl::setRespContent(const string& str, std::string&& ctype, bool tec) {
	setRespContent(str.data(), str.size(), move(ctype), tec);
}

const string& CaHttpUrlCtrl::getReqData() {
	auto *pstrm = dynamic_cast<DefReqDataStrm*>(mReqDataStrm);
	if(pstrm) {
		return pstrm->getString();
	} else {
		static string s="";
		return s;
	}
}

void CaHttpUrlCtrl::setReqDataStream(HttpBaseWriteStream* strm) {
	mReqDataStrm = strm;
}

int CaHttpUrlCtrl::writeTec(const char* ptr, size_t len) {
	int wret;
	size_t ln;
	char lenstr[50];
	ln = snprintf(lenstr, sizeof(lenstr), "%lx\r\n", len);
	assert(ln>0);
	if(mChkStrm && mChkStrm->remain()>0) {
		goto _LEN_FAIL;
	}
	wret = mCnn->writeData(*this, lenstr, ln);
	if(wret == SEND_FAIL) {
		if(mChkStrm==nullptr) mChkStrm.reset( new HttpStringReadStream);
		goto _LEN_FAIL;
	}
	wret = mCnn->writeData(*this, ptr, len);
	if(wret == SEND_FAIL) {
		if(mChkStrm==nullptr) mChkStrm.reset( new HttpStringReadStream);
		goto _DATA_FAIL;
	}
	wret = mCnn->writeData(*this, "\r\n", 2);
	if(wret == SEND_FAIL) {
		if(mChkStrm==nullptr) mChkStrm.reset( new HttpStringReadStream);
		goto _ENDL_FAIL;
	}
	return wret;

_LEN_FAIL:
	mChkStrm->store(lenstr, ln);
_DATA_FAIL:
	if(len>0) mChkStrm->store(ptr, len);
_ENDL_FAIL:
	mChkStrm->store("\r\n", 2);
	return SEND_PENDING;
}

const string& CaHttpUrlCtrl::getReqUrlStr() {
	return mReq.getReqMsg().getUrlStr();
}

int CaHttpUrlCtrl::sendString(const string& str) {
	return sendData(str.data(), str.size());
}

void CaHttpUrlCtrl::procIncomingReqMsg(ServCnn* pcnn, CaHttpMsg&& msg, bool msgonly) {
	mCnn = pcnn;
	mReq.setReqMsg(move(msg));
	OnHttpReqMsgHdr();
	if(mReqDataStrm == nullptr && mReq.getReqMsg().getContentLenInt() != 0) {
		// set default data stream
		mDefStrm.reset( new DefReqDataStrm(40*1024*1024));
		mReqDataStrm = mDefStrm.get();
	}
	if (msgonly) {
		if(mReqDataStrm) mReqDataStrm->end();
		OnHttpReqMsg();
	}
}

void CaHttpUrlCtrl::procInReqData(string&& data, bool dataend) {
	assert(mReqDataStrm!=nullptr);
	ald("proc incoming req data, size=%d, end=%d", data.size(), dataend);
	auto wret = mReqDataStrm->write(data.data(), data.size());
	if(wret>=0) {
		if(dataend) {
			if(mReqDataStrm) mReqDataStrm->end();
			OnHttpReqMsg();
		}
	} else {
		//ale("### request data too large, use stream data object for large data.");
		//SET_SHUTDOWN();
		mCnn->postCloseConnection();
	}
}
}
