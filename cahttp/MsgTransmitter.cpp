/*
 * MsgTransmitter.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE

#include "FilePacketBuf.h"
#include "StringPacketBuf.h"
#include "MsgTransmitter.h"
#include "BytePacketBuf.h"
#include "flog.h"

#define cle(fmt, ...) ale("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define clw(fmt, ...) alw("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define cln(fmt, ...) aln("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define cli(fmt, ...) ali("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define cld(fmt, ...) ald("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)
#define clv(fmt, ...) alv("h:%03d " fmt , mTxChannel, ## __VA_ARGS__)


namespace cahttp {

MsgTransmitter::MsgTransmitter() {
	mpCnn = nullptr;
	mTxChannel = 0;
	mStatus.val = 0;
	mSendDataCnt = 0;
	mContentLen = 0;
	mRecvDataCnt = 0;
	mTxChannel=0;
}

MsgTransmitter::~MsgTransmitter() {
}

int MsgTransmitter::open(BaseConnection& cnn, std::function<void(TR)> lis) {
	mLis = lis;
	mpCnn = &cnn;
	mTxChannel = mpCnn->openTxCh([this](BaseConnection::CH_E evt) {
		if(evt == BaseConnection::CH_WRITABLE) {
			procOnWritable();
		} else if(evt == BaseConnection::CH_CLOSED) {
			mTxChannel = 0;
			mLis(TR::eSendFail);
		} else {
			assert(0);
		}
		return 0;
	});
	ald("open tx channel=%d", mTxChannel);
	return (!mTxChannel);
}

int MsgTransmitter::procOnWritable() {
	alv("on writable");
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
				ret = mpCnn->send(mTxChannel, tmp, n);
				if (ret == SR::eNext || ret == SR::eFail) {
//					assert(ret == SEND_RESULT::SEND_FAIL);
					ald("*** chunk length write error");
					stackTeByteBuf(buf.second, buf.first, true, true, true, true);
					pktbuf->consume();
					goto END_SEND;
					break;
				}
			}
			ret = mpCnn->send(mTxChannel, buf.second, buf.first);
			if (ret == SR::eOk || ret==SR::ePending ) {
				if (mStatus.te && pktbuf->getType() == 1) {
					// writing chunk tail
					ret = mpCnn->send(mTxChannel, "\r\n", 2);
					if (ret == SR::eNext || ret == SR::eFail) {
						assert(ret != SR::eFail);
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
		if (mStatus.final && mStatus.se) {
			mLis(TR::eSendOk);
		} else {
			if(mStatus.phase) {
				mLis(TR::eDataNeeded);
			}
		}
	}
	END_SEND: ;
	return 0;
}


int MsgTransmitter::sendMsg(BaseMsg& msg) {
	if(mStatus.phase) {
		cle("### not sending msg phase");
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
	auto sret = mpCnn->send(mTxChannel, s.data(), s.size());
	if(msg.getContentLen()>0 || msg.getTransferEncoding()) {
		mStatus.phase = 1;
	}
	if(sret == SR::eOk) {
		if(mStatus.se) {
			mLis(MsgTransmitter::eSendOk);
		}
	} else if(sret == SR::eNext) {
		stackSendBuf(move(s), 0);
	} else if(sret == SR::eFail) {
		ale("### send msg error");
		mLis(MsgTransmitter::eSendFail);
		assert(0);
		return -1;
	}
	return 0;
}

int MsgTransmitter::sendContent(const std::string& data) {
	return sendContent(data.data(), data.size());
}

int MsgTransmitter::sendContent(const char* ptr, size_t len) {
	auto r = sendData(ptr, len, true);
	if(r != SR::eFail) {
		if(mStatus.te) {
			endData();
		}
		if(mStatus.se && mBufList.empty()) {
			mLis(eSendOk);
		}
		return 0;
	} else {
		cle("*** can't send data, sending content is already complete.");
		return -1;
	}
}

int MsgTransmitter::sendContent(std::unique_ptr<PacketBuf> upbuf) {
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
		cle("### mismatch content length, msg_len=%ld, sending_len=%ld", mContentLen, upbuf->remain());
		return -1;
	}

}


void MsgTransmitter::endData() {
	if(mStatus.te && !mStatus.se) {
		mStatus.phase=0;
		if(mStatus.final) { // TODO: request case ???
			mStatus.se = 1;
		}
		if(mBufList.empty()==true) {
			auto ret = mpCnn->send(mTxChannel, "0\r\n\r\n", 5);
			if(ret==SR::eOk || ret == SR::ePending) {
				return;
			}
		}
		stackSendBuf("0\r\n\r\n", 5, 0);
	}
}


SR MsgTransmitter::sendData(const char* ptr, size_t len, bool buffering) {
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

void MsgTransmitter::stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail, bool front) {
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


void MsgTransmitter::stackSendBuf(std::string&& s, int type) {
	auto *pbuf = new StringPacketBuf;
	pbuf->setType(type);
	pbuf->setString(move(s));
	mBufList.emplace_back(pbuf);
}

void MsgTransmitter::close() {
	if(mTxChannel) {
		mpCnn->endTxCh(mTxChannel); mTxChannel=0;
//		mpCnn->remoteTxChannel(mTxChannel);
	}

	mStatus.val = 0;
}

int MsgTransmitter::sendContentFile(const char* path) {
	auto pbuf = std::unique_ptr<FilePacketBuf>(new FilePacketBuf);
	auto r = pbuf->open(path);
	if(!r) {
		r = sendContent(move(pbuf));
	}
	return r;
}

void MsgTransmitter::stackSendBuf(const char* ptr, size_t len, int type) {
	auto *pbuf = new BytePacketBuf;
	pbuf->setType(type);
	pbuf->setData(ptr, len);
	mBufList.emplace_back(pbuf);
}




void MsgTransmitter::reserveWrite() {
	if(!mStatus.se && mTxChannel) {
		mpCnn->reserveWrite();
	}
}

} /* namespace cahttp */

