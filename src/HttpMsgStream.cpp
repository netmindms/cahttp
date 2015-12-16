/*
 * HttpMsgStream.cpp
 *
 *  Created on: Jul 27, 2015
 *      Author: netmind
 */

#include <assert.h>
#include "HttpMsgStream.h"
namespace cahttp {
#define BIT_EOS 1
#define BIT_TEC 2

#define F_EOS() (mFlag.test(BIT_EOS))
#define F_TEC() (mFlag.test(BIT_TEC))

#define SET_EOS() (mFlag.set(BIT_EOS, 1))
#define SET_TEC(A) (mFlag.set(BIT_TEC, A))


using namespace std;

HttpMsgStream::HttpMsgStream() {
	mChunkSize = 4 * 1024;
	mDataStrm = nullptr;
	mContentLen = 0;
	mReadDataCnt = 0;
	mFlag = 0;
}

HttpMsgStream::~HttpMsgStream() {
}

void HttpMsgStream::setData(string&& data) {
	auto &strm = *new HttpStringReadStream;
	strm.setString(move(data));
	mDataStrm = &strm;
	mDefDataStrm.reset(&strm);
}

void HttpMsgStream::setChunkSize(size_t size) {
	mChunkSize = size;
}

void HttpMsgStream::setMsg(CaHttpMsg& msg, HttpBaseReadStream* datastrm, bool tec) {
	if(tec) {
		mContentLen = msg.getContentLenInt();
		SET_TEC(tec);
	} else {
//		mContentLen = -1;
		mContentLen = msg.getContentLenInt();
	}
	mHdrStrm.setString(msg.serialize());
	mDataStrm = datastrm;
}

size_t HttpMsgStream::remain() {
	auto rs = mHdrStrm.remain();
	if (mDataStrm == nullptr) {
		return rs;
	}
	else {
		if (mDataStrm->remain() < 0) {
			return -1;
		}
		else {
			return (rs + mDataStrm->remain());
		}
	}
}

void HttpMsgStream::setDataStream(HttpBaseReadStream *datastrm) {
	mDataStrm = datastrm;
}

std::pair<const char*, int64_t> HttpMsgStream::getDataPtr() {
	if(mHdrStrm.remain()>0) {
		auto dp = mHdrStrm.getDataPtr();
		return dp;
	} else if(mDataStrm){
		if(!F_TEC()) {
			return mDataStrm->getDataPtr();
		} else {
			if(mTecPartBuf.empty()==false) {
				return {mTecPartBuf.data(), mTecPartBuf.size()};
			} else {
				if(F_EOS()) {
					return {nullptr, 0};
				}
				auto dp = mDataStrm->getDataPtr();
				if(dp.second > 0) {
					char buf[30];
					snprintf(buf, sizeof(buf), "%x\r\n", (uint32_t)dp.second);
					mTecPartBuf = buf;
					mTecPartBuf.append(dp.first, dp.second);
					mTecPartBuf += "\r\n";
					mDataStrm->consume(dp.second);
					if(mDataStrm->remain()==0) {
						mTecPartBuf += "0\r\n\r\n";
						SET_EOS();
					}
					return {mTecPartBuf.data(), mTecPartBuf.size()};
				} else if(dp.second ==0) {
					mTecPartBuf += "0\r\n\r\n";
					SET_EOS();
					return {mTecPartBuf.data(), mTecPartBuf.size()};
				} else {
					return {nullptr, -1};
				}
			}
		}
	} else {
#if 0
		return {nullptr, -1};
#else
		if(!F_TEC()) {
			return {nullptr, mContentLen-mReadDataCnt};
		} else {
			return {nullptr, -1};
		}
#endif
	}
}

void HttpMsgStream::consume(size_t len) {
	if(mHdrStrm.remain()>0) {
		mHdrStrm.consume(len);
	} else if(mDataStrm) {
		mReadDataCnt += len;
		if(!F_TEC()) {
			mDataStrm->consume(len);
		} else {
			assert(mTecPartBuf.size()==len);
			mTecPartBuf.clear();
		}
	}
}

}
