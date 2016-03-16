/*
 * MsgSender.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE

#include "FilePacketBuf.h"
#include "StringPacketBuf.h"
#include "MsgSender.h"
#include "BytePacketBuf.h"
#include "flog.h"

#if 0
#define cle(fmt, ...) ale("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define clw(fmt, ...) alw("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define cln(fmt, ...) aln("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define cli(fmt, ...) ali("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define cld(fmt, ...) ald("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define clv(fmt, ...) alv("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#endif

namespace cahttp {

MsgSender::MsgSender() {
	mpCnn = nullptr;
	mStatus.val = 0;
	mSendDataCnt = 0;
	mContentLen = 0;
	mRecvDataCnt = 0;
//	mTxChannel=0;
}

MsgSender::~MsgSender() {
}

int MsgSender::open(BaseCnn& cnn) {
	mpCnn = &cnn;
	ald("open msg sender");
	return 0;
}

MsgSender::TR MsgSender::procOnWritable() {
	alv("on writable");
	TR res=eMsgContinue;
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
				ret = mpCnn->send(tmp, n);
				if (ret == SR::eNext || ret == SR::eFail) {
//					assert(ret == SEND_RESULT::SEND_FAIL);
					ald("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true, true);
					pktbuf->consume();
					res = TR::eMsgContinue;
					goto END_SEND;
					break;
				}
			}
			ret = mpCnn->send(buf.second, buf.first);
			if (ret == SR::eOk || ret==SR::ePending ) {
				if (mStatus.te && pktbuf->getType() == 1) {
					// writing chunk tail
					ret = mpCnn->send("\r\n", 2);
					if (ret == SR::eNext || ret == SR::eFail) {
						assert(ret != SR::eFail);
						ald("*** fail writing chunk ending line");
						stackTeByteBuf(nullptr, 0, false, false, true, true);
						pktbuf->consume();
						res = TR::eMsgContinue;
						goto END_SEND;
						break;
					}
				}
				pktbuf->consume();
				if (ret == SR::ePending) {
					res = TR::eMsgContinue;
					goto END_SEND;
					break;
				}
			} else {
				ali("*** fail writing chunk body, ...");
				if (mStatus.te && pktbuf->getType() == 1) {
					stackTeByteBuf(buf.second, buf.first, false, true, true, true);
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
	if (mBufList.empty() == true) {
		if (mStatus.final && mStatus.se) {
//			mLis(TR::eSendOk);
			res = TR::eMsgSendOk;
			mpCnn->sendEnd();
		} else {
			if(mStatus.phase) {
//				mLis(TR::eDataNeeded);
				res = TR::eMsgDataNeeded;
			}
		}
	}
	END_SEND:
	return res;
}


int MsgSender::sendMsg(BaseMsg& msg) {
	if(mStatus.phase) {
		ale("### not sending msg phase");
		return -1;
	}
	mSendDataCnt = 0;
	mStatus.te = (msg.getTransferEncoding());
	mContentLen = msg.getContentLen();
	if(msg.getMsgType()==BaseMsg::REQUEST || msg.getRespStatus() >= 200) {
		mStatus.final = 1;
	}
	if(mStatus.final && !mStatus.te && mContentLen==0) {
		mStatus.se = 1;
	}
	auto s = msg.serialize();
	auto sret = mpCnn->send(s.data(), s.size());
	if(msg.getContentLen()>0 || msg.getTransferEncoding()) {
		mStatus.phase = 1;
	}
	if(sret == SR::eOk) {
		if(mStatus.se) {
//			mLis(MsgSender::eSendOk);
			mpCnn->sendEnd();
		}
	} else if(sret == SR::eNext) {
		stackSendBuf(move(s), 0);
	} else if(sret == SR::eFail) {
		ale("### send msg error");
//		mLis(MsgSender::eSendFail);
		mpCnn->sendEnd();
		assert(0);
		return -1;
	}
	return 0;
}

int MsgSender::sendContent(const std::string& data) {
	return sendContent(data.data(), data.size());
}

int MsgSender::sendContent(const char* ptr, size_t len) {
	auto r = sendData(ptr, len, true);
	if(r != SR::eFail) {
		if(mStatus.te) {
			endData();
		}
		if(mStatus.se && mBufList.empty()) {
//			mLis(eSendOk);
			mpCnn->sendEnd();
		}
		return 0;
	} else {
		ale("*** can't send data, sending content is already complete.");
		return -1;
	}
}

int MsgSender::sendContent(std::unique_ptr<PacketBuf> upbuf) {
	if(!mStatus.se && mStatus.phase ) {
		upbuf->setType(mStatus.te);
		mSendDataCnt += upbuf->remain();
		mBufList.emplace_back(move(upbuf));
		if(mStatus.te) {
			endData();
		} else {
			if(mStatus.final) {
				mStatus.se = 1;
			}
		}
		mStatus.phase = 0;
		mpCnn->reserveWrite();
		return 0;
	} else {
		ale("### mismatch content length, msg_len=%ld, sending_len=%ld", mContentLen, upbuf->remain());
		return -1;
	}

}


void MsgSender::endData() {
	if(mStatus.te && !mStatus.se) {
		mStatus.phase=0;
		if(mStatus.final) { // TODO: request case ???
			mStatus.se = 1;
		}
		if(mBufList.empty()==true) {
			auto ret = mpCnn->send("0\r\n\r\n", 5);
			if(ret==SR::eOk || ret == SR::ePending) {
				return;
			}
		}
		stackSendBuf("0\r\n\r\n", 5, 0);
	}
}


SR MsgSender::sendData(const char* ptr, size_t len, bool buffering) {
	if(mStatus.se || !mStatus.phase) {
		return eFail;
	}

	if(mBufList.size()) {
		if(buffering) {
			stackSendBuf(ptr, len, mStatus.te);
			mSendDataCnt += len;
			if(!mStatus.te && mSendDataCnt == mContentLen) {
				mStatus.phase = 0;
				if(mStatus.final) {
					mStatus.se = 1;
				}
			}
			return SR::ePending;
		} else {
			return SR::eNext;
		}
	}

	SR ret;
	if(!mStatus.te) {
		ret = mpCnn->send(ptr, len);
		if(ret == SR::eOk || SR::ePending) {
			mSendDataCnt += len	;
			if(!mStatus.te && mSendDataCnt == mContentLen) {
				mStatus.phase = 0;
				if(mStatus.final) {
					mStatus.se = 1;
				}
			}
		}
	} else {
		char tmp[20];
		auto n = sprintf(tmp, "%lx\r\n", (size_t) len);
		ret = mpCnn->send(tmp, n);
		if(ret == SR::eOk) {
			ret = mpCnn->send(ptr, len);
			if(ret == SR::eOk) {
				ret = mpCnn->send("\r\n", 2);
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

void MsgSender::stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail, bool front) {
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


void MsgSender::stackSendBuf(std::string&& s, int type) {
	auto *pbuf = new StringPacketBuf;
	pbuf->setType(type);
	pbuf->setString(move(s));
	mBufList.emplace_back(pbuf);
}

void MsgSender::close() {
//	if(mTxChannel) {
//		mpCnn->endTxCh(mTxChannel); mTxChannel=0;
//	}

	mStatus.val = 0;
}

int MsgSender::sendContentFile(const char* path) {
	auto pbuf = std::unique_ptr<FilePacketBuf>(new FilePacketBuf);
	auto r = pbuf->open(path);
	if(!r) {
		r = sendContent(move(pbuf));
	}
	return r;
}

void MsgSender::stackSendBuf(const char* ptr, size_t len, int type) {
	auto *pbuf = new BytePacketBuf;
	pbuf->setType(type);
	pbuf->setData(ptr, len);
	mBufList.emplace_back(pbuf);
}




void MsgSender::reserveWrite() {
	if(!mStatus.se) {
		mpCnn->reserveWrite();
	}
}

} /* namespace cahttp */
