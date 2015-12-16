
/*
 * CaHttpReq.cpp
 *
 *  Created on: Apr 19, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG
#include "flog.h"
#include "CaHttpCommon.h"
#include "CaHttpUrlParser.h"
#include "HttpStringWriteStream.h"
#include "CaHttpReq.h"
#include "HttpCnn.h"
#include "CaHttpReqMan.h"
namespace cahttp {
#define MAX_RECV_BUF (2.5*1024*1024)

#define BIT_SEND_PAUSE 1
#define BIT_MANUAL_SEND 2
#define BIT_TEC 3
#define BIT_SEND_EOS 4
#define BIT_RESP_COMP 5
#define BIT_REQ_ACTIVE 6

#define F_MANUAL_SEND() (mStatusFlag.test(BIT_MANUAL_SEND))
#define F_SEND_PAUSE() (mStatusFlag.test(BIT_SEND_PAUSE))
#define F_TEC() (mStatusFlag.test(BIT_TEC))
#define F_SEND_EOS() (mStatusFlag.test(BIT_SEND_EOS))
#define F_RESP_COMP() (mStatusFlag.test(BIT_RESP_COMP))
#define F_REQ_ACTIVE() (mStatusFlag.test(BIT_REQ_ACTIVE))

#define SET_SEND_PAUSE(A) (mStatusFlag.set(BIT_SEND_PAUSE, A))
#define SET_MANUAL_SEND(A) (mStatusFlag.set(BIT_MANUAL_SEND, A))
#define SET_TEC(A) (mStatusFlag.set(BIT_TEC, A))
#define SET_SEND_EOS() (mStatusFlag.set(BIT_SEND_EOS, 1))
#define SET_RESP_COMP() (mStatusFlag.set(BIT_RESP_COMP, 1))
#define SET_REQ_ACTIVE(A) (mStatusFlag.set(BIT_REQ_ACTIVE, A))

using namespace std;

CaHttpReq::CaHttpReq() {
	mpReqMan = nullptr;
	mHostIp = 0;
	mHostPort = 80;
	mMaxRecvBufSize = MAX_RECV_BUF;
	mReqContentLen = 0;
	mpCnn = nullptr;
	mSendWriteCnt = 0;
	mpReqDataStrm = nullptr;
	mpRespDataStrm = nullptr;
	mStatusFlag = 0;
}

CaHttpReq::~CaHttpReq() {
}



int CaHttpReq::getRespCode() {
	return mRespMsg.getRespCode();
}

long CaHttpReq::getRespContentLen() {
	return mRespMsg.getContentLenInt();
}

void CaHttpReq::getRespData(string& bufstr) {
	bufstr = move(mRecvDataBuf);
	mRecvDataBuf.clear();
}

const string& CaHttpReq::getRespData() {
	return mRecvDataBuf;
}


CaHttpMsg& CaHttpReq::getReqMsg() {
	return mReqMsg;
}

CaHttpMsg& CaHttpReq::getRespMsg() {
	return mRespMsg;
}

void CaHttpReq::setOnListener(Lis lis) {
	mLis = lis;
}

int64_t CaHttpReq::getReqContentLen() {
	auto cs = mReqMsg.getHdrOne(CAS::HS_CONTENT_LEN);
	if (!cs.empty()) {
		return stol(cs);
	}
	cs = mReqMsg.getHdrOne(CAS::HS_TRANSFER_ENC);
	if (!cs.empty()) {
		return -1;
	}
	return 0;
}

int CaHttpReq::getMethod() {
	return mReqMsg.getMethod();
}

void CaHttpReq::setReqMsg(CaHttpMsg&& msg) {
	mReqMsg = move(msg);
}

void CaHttpReq::setRespDataStream(HttpBaseWriteStream *pstrm) {
	mpRespDataStrm = pstrm;
}

HttpBaseWriteStream* CaHttpReq::getHttpRespStream() {
	return mpRespDataStrm;
}

int CaHttpReq::request(const string& method) {
	ali("request, method=%s, url=%s", method, mReqMsg.getUrlStr());
	mReqMsg.setMethod(method);

	string hostname;
	if(!mHostIp) {
		CaHttpUrlParser parser;
		parser.parse(mReqMsg.getUrlStr());
		auto ip = get_ip_from_hostname(parser.hostName);
		mHostIp = ip;
		if(parser.port != "") {
			mHostPort = stoi(parser.port);
		} else if(parser.scheme == "http") {
			mHostPort = 80;
		} else if(parser.scheme == "https") {
			mHostPort = 443;
		} else {
			mHostPort = 0;
		}
		hostname = parser.hostName;
		if(parser.scheme == "http") {
			if(mHostPort != 80) hostname += ":" + parser.port;
		} else if(parser.scheme == "https") {
			if(mHostPort != 443) hostname += ":" + parser.port;
		}
	}
	if (hostname.empty() == false) {
		mReqMsg.setHdr(CAS::HS_HOST, hostname);
	}
	mReqMsg.setHdr(CAS::HS_USER_AGENT, "cahttp");
//	mReqMsg.setHdr("Accept", "*/*");

	mReqMsg.setHdr(CAS::HS_DATE, get_http_cur_date_str());

	if(!F_TEC()) {
		mReqMsg.setHdr(CAS::HS_CONTENT_LEN, to_string(mReqContentLen));
		mReqMsg.setContentLenInt(mReqContentLen);
	} else {
		mReqContentLen = -1;
		mReqMsg.setHdr(CAS::HS_TRANSFER_ENC, "chunked");
	}

	if(mpReqMan) {
		mpCnn = mpReqMan->getConnection(mHostIp, mHostPort);
	}
	if(mpCnn==nullptr) {
		mpCnn = new HttpCnn;
		mpuCnn.reset(mpCnn);
		mpCnn->setHostIpAddr(mHostIp, mHostPort);
	}

	mReqMsgStrm.setMsg(mReqMsg, mpReqDataStrm, F_TEC());
	if (mpReqDataStrm == nullptr) {
		if(mReqContentLen != 0) SET_MANUAL_SEND(1);
		mSendWriteCnt = 0;
	}

	HttpCnn::Lis lis;
	if(!F_MANUAL_SEND()) {
		lis = [this](HttpCnn::CE event){
			if(event == HttpCnn::CE_RECV_MSGHDR) {
				mLis(*this, CaHttpReq::HTTP_RESP_HDR, 0);
				return 0;
			} else if(event == HttpCnn::CE_RECV_MSG) {
				SET_RESP_COMP();
				mRespMsg = mpCnn->fetchRecvMsg();
				SET_REQ_ACTIVE(0);
				mLis(*this, CaHttpReq::HTTP_REQ_END, 0);
				return 0;
			} else if(event == HttpCnn::CE_CLOSED) {
				SET_REQ_ACTIVE(0);
				mLis(*this, CaHttpReq::HTTP_REQ_END, F_RESP_COMP()==1?0:-1);
			} else if(event == HttpCnn::CE_UNDERRUN) {
				SET_SEND_PAUSE(1);
				mLis(*this, CaHttpReq::HTTP_REQ_DATA_UNDERRUN, 0);
				SET_SEND_PAUSE(0);
			} else if(event == HttpCnn::CE_FAIL) {
				SET_REQ_ACTIVE(0);
				mLis(*this, CaHttpReq::HTTP_REQ_END, -1);
			} else {
				assert(0);
				return 0;
			}
			return 0;
		};
	} else {
		lis = [this](HttpCnn::CE event){
			if(event == HttpCnn::CE_RECV_MSGHDR) {
				mLis(*this, CaHttpReq::HTTP_RESP_HDR, 0);
				return 0;
			} else if(event == HttpCnn::CE_RECV_MSG) {
				SET_RESP_COMP();
				SET_REQ_ACTIVE(0);
				mRespMsg = mpCnn->fetchRecvMsg();
				mLis(*this, CaHttpReq::HTTP_REQ_END, 0);
				return 0;
			} else if(event == HttpCnn::CE_CLOSED) {
				SET_REQ_ACTIVE(0);
				mLis(*this, CaHttpReq::HTTP_REQ_END, F_RESP_COMP()==1?0:-1);
			} else if(event == HttpCnn::CE_UNDERRUN) {
				SET_SEND_PAUSE(1);
				mLis(*this, CaHttpReq::HTTP_REQ_DATA_UNDERRUN, 0);
//				SET_SEND_PAUSE(0);
				if(!F_SEND_PAUSE()) {
					return 1;
				} else {
					return 0;
				}
			} else if(event == HttpCnn::CE_FAIL) {
				SET_REQ_ACTIVE(0);
				mLis(*this, CaHttpReq::HTTP_REQ_END, -1);
			} else {
				assert(0);
				return 0;
			}
			return 0;
		};
	}
	mpCnn->addMsgStream(&mReqMsgStrm, lis, [this](string &&data, int data_status) {
		if(mpRespDataStrm==nullptr) {
			if(mRecvDataBuf.size()+data.size() <= mMaxRecvBufSize) {
				mRecvDataBuf.append(data);
			} else {
				// TODO: fail
			}
		} else {
			mpRespDataStrm->write(data.data(), data.size());
		}
	});

	SET_REQ_ACTIVE(1);
	mpCnn->transfer();
	return 0;
}

void CaHttpReq::setReqContent(const string& data, const string& ctype) {
	auto &strm = *new HttpStringReadStream;
	mReqContentLen = data.size();
	strm.setString(string(data));
	mReqMsg.setHdr(CAS::HS_CONTENT_TYPE, string(ctype));
	mpReqDataStrm = &strm;
	mupDefReqDataStrm.reset(&strm);
}

void CaHttpReq::setReqContent(const char* ptr, size_t len, const string& ctype) {
	setReqContent(string(ptr, len), ctype);
}

void CaHttpReq::setReqContent(HttpBaseReadStream *pstrm, const string& ctype) {
	assert(pstrm);
	mReqMsg.setHdr(CAS::HS_CONTENT_TYPE, ctype);
	mpReqDataStrm = pstrm;
	mReqContentLen = pstrm->remain();
}

void CaHttpReq::setReqContent(size_t len, const string& ctype) {
	mReqContentLen = len;
	mReqMsg.setHdr(CAS::HS_CONTENT_TYPE, ctype);
}


void CaHttpReq::setUrl(const string& urlstr) {
	mReqMsg.setUrl(urlstr.data(), urlstr.size());
}

int CaHttpReq::sendData(const char* ptr, size_t len) {

#if 1
	if(F_SEND_EOS()) {
		return -3;
	}

	if(F_MANUAL_SEND()) {
		int wret;
		if(!F_TEC()) {
			if(mSendWriteCnt+(ssize_t)len<=mReqContentLen) {
				wret = mpCnn->writeData(ptr, len);
			} else {
				wret = SEND_FAIL;
			}
			if(wret != SEND_FAIL) {
				if(len>0) SET_SEND_PAUSE(0);
				mSendWriteCnt += len;
				if(mSendWriteCnt >= mReqContentLen) {
					SET_SEND_EOS();
				}
				return 0;
			}
		} else {
			if(len == 0) {
				SET_SEND_EOS();
			}
			wret = writeTec(ptr, len);
			if(wret != SEND_FAIL) {
				SET_SEND_PAUSE(0);
				return 0;
			} else {
				return -1;
			}
		}

		return -1;
	} else {
		assert(0);
		return -1;
	}
#else
	if(!mpReqDataStrm) {
		if(mReqContentLen<0 || (mSendWriteCnt+len <= mReqContentLen) ) {
			auto wret = mpCnn->writeData(ptr, len);
			if(wret != SEND_FAIL) {
				mSendWriteCnt += len;
				return 0;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
	} else {
		assert(0);
		return -1;
	}
#endif
}

void CaHttpReq::enableTransferChunked() {
	SET_TEC(1);
}


#if 0
int CaHttpReq::request(const string& method, const string& path, HdrList &&hdrlist, HttpBaseReadStream *sstrm, upHttpBaseWriteStream rstrm, Lis lis) {
	close();
	HttpCnn *pcnn;
	if (mCnnHandle == 0) {
		if(mpCnnMan==nullptr) {
			assert(0);
			return -100;
		}
		pcnn = &(mpCnnMan->newConnection(mHostIp, mHostPort, 0));
		mCnnHandle = pcnn->getHandle();
	}
	else {
		pcnn = mpCnnMan->getConnection(mCnnHandle);
		if (pcnn == nullptr) {
			ale("### Error: invalid connection handle=%0x", mCnnHandle);
			return -100;
		}
	}
	mLis = lis;
	mReqMsg.setHdrList(move(hdrlist));
	mReqMsg.setPath(path);
	mReqMsg.setMethod(string(method));
	auto &httpstrm = *new HttpMsgStream;
	httpstrm.setHdrBuf(mReqMsg.serialize());
	if (sstrm) {
		httpstrm.setDataStream(move(sstrm));
	}
	if (rstrm) {
		mRespDataType = 2;
		mpRespDataStrm = move(rstrm);
	}
	else {
		mRespDataType = 0;
	}
	pcnn->setHostIpAddr(mHostIp, mHostPort);
	pcnn->addMsgStream(&httpstrm, [this](HttpCnn::CE event){
		if(event == HttpCnn::CE_RECV_MSGHDR) {
			return 0;
		} else if(event == HttpCnn::CE_RECV_MSG) {
			mLis(HRE::HTTP_REQ_END, *this);
			return 0;
		} else if(event == HttpCnn::CE_CLOSED) {
			return 0;
		} else if(event == HttpCnn::CE_UNDERRUN) {
			return 0;
		} else {
			assert(0);
			return 0;
		}
		return 0;
	}, [this](string &&data, int data_status) {
		if(mpRespDataStrm==nullptr) {
			if(recvDataBuf.size()+data.size() <= mMaxRecvBufSize) {
				recvDataBuf.append(data);
			} else {
				// TODO: fail
			}
		}
	});
	pcnn->transfer();
}
#endif

int CaHttpReq::request(const string& method, const string& path, Lis lis) {
	mLis = lis;
	mReqMsg.setUrl(path.data(), path.size());
	return request(method);
}

void CaHttpReq::setRemoteHostAddr(uint32_t ip, int port) {
	mHostIp = ip;
	mHostPort = port;
}

void CaHttpReq::setRemoteHostAddr(const char* ip, int port) {
	mHostIp = inet_addr(ip);
	mHostPort = port;
}

void CaHttpReq::setRemoteHostAddr(const string& ip, int port) {
	setRemoteHostAddr(ip.data(), port);
}

void CaHttpReq::close() {
	if(mpReqMan) {
		mpReqMan->freeRequest(this);
		mpReqMan = nullptr;
	}
	if(mpuCnn) {
		mpuCnn->close();
	}

	mStatusFlag = 0;
}

void CaHttpReq::setReqHdr(const string& name, const string& val) {
	mReqMsg.setHdr(string(name), val);
}

int CaHttpReq::clearRequest() {
	if(F_REQ_ACTIVE()) {
		ale("### request still active,...");
		return -1;
	}
	mReqContentLen = 0;
	mSendWriteCnt = 0;
	mReqMsg.clear();
	mpRespDataStrm = nullptr;
	mRespMsg.clear();
	mRecvDataBuf.clear();
	mHostIp = 0;
	mHostPort = 0;
	mpReqDataStrm = nullptr;
	mupDefReqDataStrm = nullptr;
	mpReqMan = nullptr;
	mpCnn = nullptr;
	mpuCnn = nullptr;
	mStatusFlag = 0;
	mChunkedBuf.clear();
	return 0;
}

int CaHttpReq::writeTec(const char* ptr, size_t len) {
	char buf[30];
	snprintf(buf, sizeof(buf), "%x\r\n", len);
	mChunkedBuf.append(buf);
	if(len>0) mChunkedBuf.append(ptr, len);
	mChunkedBuf.append("\r\n");
	auto wret = mpCnn->writeData(mChunkedBuf.data(), mChunkedBuf.size());
	if(wret != SEND_FAIL) {
		mChunkedBuf.clear();
		return 0;
	} else {
		return -1;
	}
}

void CaHttpReq::setReqMan(CaHttpReqMan* preqman) {
	mpReqMan = preqman;
}

int CaHttpReq::request_get(const string& path, Lis lis) {
	return request(CAS::MS_GET, path, lis);
}

int CaHttpReq::request_post(const std::string& path, Lis lis) {
	return request(CAS::MS_POST, path, lis);
}
}
