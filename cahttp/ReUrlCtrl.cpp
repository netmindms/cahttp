
/*
 * ReUrlCtrl.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "FilePacketBuf.h"
#include "BytePacketBuf.h"
#include "StringPacketBuf.h"

#include "ReUrlCtrl.h"
#include "CaHttpCommon.h"
#include "ReSvrCnn.h"
#include "ext/nmdutil/FileUtil.h"
#include "ext/nmdutil/etcutil.h"
#include "ext/nmdutil/strutil.h"
#include "flog.h"


#define cln(fmt, ...) aln("h:%03d " fmt , mHandle, ## __VA_ARGS__)
#define cli(fmt, ...) ali("h:%03d " fmt , mHandle, ## __VA_ARGS__)
#define cld(fmt, ...) ald("h:%03d " fmt , mHandle, ## __VA_ARGS__)
#define clv(fmt, ...) alv("h:%03d " fmt , mHandle, ## __VA_ARGS__)

using namespace std;
using namespace cahttpu;

namespace cahttp {

ReUrlCtrl::ReUrlCtrl() {
	mpCnn = nullptr;
	mHandle = 0;
	mStatus.val = 0;
	mSendDataCnt = 0;
	mContentLen = 0;
	mpServCnn = nullptr;
	mRecvDataCnt = 0;
	mRxChannel=0;
}

ReUrlCtrl::~ReUrlCtrl() {
//	if (mpReqMsg) {
//		delete mpReqMsg;
//	}
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

void ReUrlCtrl::OnHttpEnd(int err) {
	ald("on end...");
}

void ReUrlCtrl::init(upBaseMsg upmsg, ReSvrCnn& cnn, uint32_t hsend) {
	mupReqMsg = move(upmsg);
	mpServCnn = &cnn;
	mpCnn = mpServCnn->getConnection();
	mHandle = hsend;
	cld("new url ctrl, path=%s", mupReqMsg->getUrl());

	mRxChannel = mpCnn->openRxCh([this](BaseConnection::CH_E evt){
		if(evt == BaseConnection::CH_DATA) {
			procOnData(mpCnn->fetchData());
		} else if(evt == BaseConnection::CH_CLOSED) {
			mRxChannel = 0;
			mStatus.err = 1;
			mMsgTx.close();
		} else {
			ale("### not expected event=%d", (int)evt);
			assert(0);
		}
		return 0;
	});
	cld("open rx channel=%d", mRxChannel);

	mMsgTx.open(*mpCnn, [this](MsgTransmitter::TR evt) {
		switch(evt) {
		case MsgTransmitter::eSendOk:
			cld("response send complete.");
			if(mRxChannel)  {
				mpCnn->endRxCh(mRxChannel);
				mRxChannel = 0;
			}
			mStatus.fin = 1;
			mpServCnn->endCtrl(mHandle);
			mMsgTx.close();
			break;
		case MsgTransmitter::eDataNeeded:
			clv("data needed");
			OnHttpSendBufReady();
			break;
		case MsgTransmitter::eSendFail:
			cln("*** response send fail");
			mStatus.err = 1;
			mpServCnn->endCtrl(mHandle);
			mMsgTx.close();
			break;
		default:
			assert(0);
			break;

		}
	});
	mStatus.used = 1;
	cld("open msg transmitter, channel=%d", mMsgTx.getTxChannel());
	OnHttpReqMsgHdr(*mupReqMsg.get());
}

#if 1
int ReUrlCtrl::response(int status_code, const char *pdata, size_t data_len, const char* ctype) {
	setBasicHeader(mRespMsg, 200);
	mRespMsg.setContentLen(data_len);
	if(!mRespMsg.getTransferEncoding()) {
		mRespMsg.setContentType(ctype);
	}
	auto r = response(mRespMsg);
	if(!r) {
		if(pdata) {
			auto r = mMsgTx.sendContent(pdata, data_len);
			return r;
		}
	}
	return r;
}


int ReUrlCtrl::response(int status_code) {
	setBasicHeader(mRespMsg, status_code);
	if(mRespMsg.getTransferEncoding()==false) {
		mRespMsg.setContentLen(0);
	}
	return response(mRespMsg);
}

int ReUrlCtrl::response(int status_code, const std::string& content, const std::string& ctype) {
	setBasicHeader(mRespMsg, status_code);
	mRespMsg.setContentType(ctype);
	if(mRespMsg.getTransferEncoding()==false) {
		mRespMsg.setContentLen(content.size());
	}
	auto r = response(mRespMsg);
	if (r) {
		return r;
	}
//	return writeContent(content.data(), content.size());
	return mMsgTx.sendContent(content);
}

int ReUrlCtrl::response_file(const char* path) {
	struct stat finfo;
	auto r = stat(path, &finfo);
	if(!r && finfo.st_size>0 && S_ISREG(finfo.st_mode) && !S_ISDIR(finfo.st_mode)) {
		int status_code=200;
		setBasicHeader(mRespMsg, status_code);
		if(mRespMsg.getTransferEncoding()==false) {
			mRespMsg.setContentLen(finfo.st_size);
		}
		mRespMsg.setContentType(CAS::CT_APP_OCTET);
		response(mRespMsg);
		auto r = mMsgTx.sendContentFile(path);
		return r;
	} else {
		aln("*** invalid file=%s", path);
		mRespMsg.setContentLen(0);
		return response(404);
	}
}

#else
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
#endif

int ReUrlCtrl::response(BaseMsg& msg) {
#if 1
	cli("response: status=%d, content_len=%ld", msg.getRespStatus(), msg.getContentLen());
	auto r = mMsgTx.sendMsg(msg);
	msg.clear();
	return r;
#else
	mSendDataCnt = 0;
	mStatus.te = msg.getTransferEncoding();
	mContentLen = msg.getContentLen();
	auto s = msg.serialize();
	auto sret = mpCnn->send(mTxChannel, s.data(), s.size());
	if(sret == SR::eOk) {
		if(!mStatus.te && mContentLen==0) {
			mStatus.se = 1;
			mStatus.fin = 1;
			mpServCnn->endCtrl(mHandle);
		}
		return 0;
	} else	if (sret == SR::eNext) {
		stackSendBuf(move(s), 0);
		return 0;
	} else if (sret == SR::ePending) {
		return 0;
	} else {
		ale("### response error");
		return 1;
	}
	return 0;
#endif
}

int ReUrlCtrl::procOnData(std::string&& data) {
	alv("proc on data, size=%ld", data.size());
	if(data.size()>0) {
		OnHttpReqData(move(data));
	} else if(data.size()==0){
		ald("empty data, request message end.");
		mpCnn->endRxCh(mRxChannel); mRxChannel=0;
		OnHttpReqMsg(* mupReqMsg.get());
	}
	return 0;
}



#if 0

int ReUrlCtrl::sendHttpMsg(std::string&& msg) {
	assert(msg.size() > 0);
//	ald("sending http msg: %s", msg);
	auto ret = mpServCnn->send(mHandle, msg.data(), msg.size());
	if (ret == SR::eNext) {
		// send next
		auto *pkt = new StringPacketBuf;
		pkt->setType(1);
		pkt->setString(move(msg));
		mBufList.emplace_front();
		mBufList.front().reset(pkt);
		return -1;
	} else {
		mpCnn->reserveWrite();
		return 0;
	}
}
int ReUrlCtrl::procOnWritable() {
	ali("on writable");
	SR ret;
	for (; mBufList.empty() == false;) {
		auto *pktbuf = mBufList.front().get();
		auto buf = pktbuf->getBuf();
		alv("get buf, size=%ld", buf.first);
		if (buf.first > 0) {
			if (mStatus.te && pktbuf->getType() == 1) {
				// writing chunk head
				char tmp[20];
				auto n = sprintf(tmp, "%lx\r\n", (size_t) buf.first);
				ret = mpServCnn->send(mHandle, tmp, n);
				if (ret == SR::eNext || ret == SR::eFail) {
//					assert(ret == SEND_RESULT::SEND_FAIL);
					ald("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true, true);
					pktbuf->consume();
					goto END_SEND;
					break;
				}
			}
			ret = mpServCnn->send(mHandle, buf.second, buf.first);
			if (ret <= 0) {
				if (mStatus.te && pktbuf->getType() == 1) {
					// writing chunk tail
					ret = mpServCnn->send(mHandle, "\r\n", 2);
					if (ret == SR::eNext || ret == SR::eFail) {
						assert(ret == SR::eFail);
						ald("*** fail writing chunk ending line");
						stackTeByteBuf(nullptr, 0, false, false, true, true);
						pktbuf->consume();
						goto END_SEND;
						break;
					}
				}
				pktbuf->consume();
				if (ret == SR::ePending) {
					goto END_SEND;
					break;
				}
			} else {
				ali("*** fail writing chunk body, ...");
				if (mStatus.te && pktbuf->getType() == 1) {
					stackTeByteBuf(buf.second, buf.first, false, true, true, true);
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
		if (mStatus.se) {
			OnHttpEnd();
		} else {
			OnHttpSendBufReady();
		}
	}
	END_SEND: ;
	return 0;
}

void ReUrlCtrl::stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail, bool front) {
	auto *bf = new BytePacketBuf;
	bf->setType(0);
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

	if(front) {
		mBufList.emplace_front();
		mBufList.front().reset(bf);
	} else {
		mBufList.emplace_back();
		mBufList.back().reset(bf);
	}
}

SR ReUrlCtrl::send(const char* ptr, size_t len, bool buffering) {
	if(mStatus.se) {
		return eFail;
	} else if(mBufList.size()) {
		if(buffering) {
			stackSendBuf(ptr, len, mStatus.te);
			mSendDataCnt += len;

			return ePending;
		} else {
			return eNext;
		}
	}

	SR ret;
	if(!mStatus.te) {
		ret = mpCnn->send(mTxChannel, ptr, len);
		if(ret == SR::eOk || SR::ePending) {
			mSendDataCnt += len	;
			if(mSendDataCnt == mContentLen) {
				mStatus.se = 1;
			}
		}
	} else {
		char tmp[20];
		auto n = sprintf(tmp, "%lx\r\n", (size_t) len);
		ret = mpCnn->send(mTxChannel, tmp, n);
		if(ret == SR::eOk) {
			ret = mpCnn->send(mTxChannel, ptr, len);
			if(ret == SR::eOk) {
				ret = mpCnn->send(mTxChannel, "\r\n", 2);
				if(ret == SR::eNext) {
					stackTeByteBuf(ptr, len, false, false, true	, false);
				}
			} else if(ret == SR::eNext) {
				stackTeByteBuf(ptr, len, false, true, true, false);
			}
		} else if( ret == SR::eNext) {
			stackTeByteBuf(ptr, len, true, true ,true, false);
		}
	}
	return ret;
}

void ReUrlCtrl::stackSendBuf(std::string&& s, int type) {
	auto *pbuf = new StringPacketBuf;
	pbuf->setType(type);
	pbuf->setString(move(s));
	mBufList.emplace_back(pbuf);
}

void ReUrlCtrl::stackSendBuf(const char* ptr, size_t len, int type) {
	auto *pbuf = new BytePacketBuf;
	pbuf->setType(type);
	pbuf->setData(ptr, len);
	mBufList.emplace_back(pbuf);
}

void ReUrlCtrl::endData() {
	if(mStatus.te && !mStatus.se) {
		if(mBufList.empty()==true) {
			mStatus.se = 1;
			auto ret = mpCnn->send(mTxChannel, "0\r\n\r\n", 5);
			if(ret==SR::eOk || ret == SR::ePending) {
				return;
			}
		}
		stackSendBuf("0\r\n\r\n", 5, 0);
	}
}
int ReUrlCtrl::writeContent(const char* ptr, size_t len) {
	if (!mStatus.te && (mSendDataCnt + (int64_t)len > mContentLen)) {
		ale("### too much content size, content_size=%ld, cur_send_cnt=%ld, data_len=%ld", mContentLen, mSendDataCnt, len);
		return 1;
	}

	if (mBufList.empty() == true) {
		auto sret = mpServCnn->send(mHandle, ptr, len);
		if (sret == SR::eOk) {
			mSendDataCnt += len;
			if(!mStatus.te && mSendDataCnt >= mContentLen) {
				assert(mSendDataCnt <= mContentLen);
				mStatus.fin=1;//FS_FIN();
				mpServCnn->endCtrl(mHandle);
				return 0;
			}

		} else if (sret == SR::eNext) {
			stackSendBuf(ptr, len, 0);
		}

		if (sret != SR::eFail) {
			mSendDataCnt += len;
			return 0;
		} else {
			return 1;
		}
	} else {
		stackSendBuf(ptr, len, 0);
		return 0;
	}
}

int ReUrlCtrl::sendContent(const char* ptr, size_t len) {
	if(!mStatus.se) {
		send(ptr, len, true);
		if(mStatus.te) {
			endData();
		}
		if(mBufList.empty()) {
			ald("content sending complete. h=%d", mHandle);
			assert(mStatus.se);
			mpCnn->endTxCh(mTxChannel); mTxChannel=0;
		}
		return 0;
	} else {
		alw("*** can't send data, sending content is already complete.");
		return -1;
	}
}

#endif

void ReUrlCtrl::OnHttpSendBufReady() {
}



void ReUrlCtrl::OnHttpReqData(std::string&& data) {

}




void ReUrlCtrl::setBasicHeader(BaseMsg& msg, int status_code) {
	msg.setMsgType(BaseMsg::MSG_TYPE_E::RESPONSE);
	msg.setRespStatus(status_code);
	msg.addHdr(CAS::HS_DATE, get_http_cur_date_str());
}


bool ReUrlCtrl::isComplete() {
	return mStatus.fin; //F_FIN();
}

void ReUrlCtrl::closeChannels() {
	if(mRxChannel) {
		mpCnn->endRxCh(mRxChannel); mRxChannel=0;
	}
	mMsgTx.close();
}

} /* namespace cahttp */
