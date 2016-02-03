/*
 * ServCnn.cpp
 *
 *  Created on: Jul 19, 2015
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include "flog.h"
#include "ServCnn.h"

#include <stdlib.h>
#include "CaHttpCommon.h"
#include "CaHttpUrlParser.h"
#include "ServTask.h"
#include "CaHttpServer.h"
#include "HttpServCnnCtx.h"
#include "CaHttp404UrlCtrl.h"

using namespace std;

namespace cahttp {
enum {
	LM_DISCONNECT=(EDM_USER+100),
	LM_TEST,
};

#define DUMMY_CTRL(A) (mDummyCtrl = move(A))
#define CNN_TIMEOUT (20*1000)
#define INTERNAL_CLEAR() { \
	mReqCtrl = nullptr; \
	mpCtx = nullptr; \
	mRecvBuf = nullptr; \
	mBufSize = 0;\
	mRespCtrl = nullptr; \
	mTimerWork = 0;\
	mHandle = 0;\
	mIsPipeliningSupport = false;\
	mParsingStop = false;\
	mTimerUpdate = 0; \
	mPauseSend = false; \
}

ServCnn::ServCnn() {
	ali("Service connection constructed ");
	INTERNAL_CLEAR()
	;
}

ServCnn::ServCnn(const ServCnn& other) {
	ald("ServCnn ref const...");
	INTERNAL_CLEAR()
	;
}

ServCnn::ServCnn(int handle) {
	ald("ServCnn handle const...");
	INTERNAL_CLEAR()
	;
	mHandle = handle;
}

ServCnn::~ServCnn() {
	ald("Service connection desctructed...handle=%d", mHandle);
	if (mRecvBuf) {
		free(mRecvBuf);
	}
}

int ServCnn::open(int fd, uint32_t handle, HttpServCnnCtx *pctx) {
	if (mSock.getFd() > 0) {
		return 0;
	}
	ald("open: handle=%0x, fd=%d", handle, fd);
	init_local_msgque();

	mpCtx = pctx;
//	mParsingStop = false;
	mHandle = handle;
	mMsgFrame.init(true);
//	mTask = (ServTask*) EdTask::getCurrentTask();
	mBufSize = 4096;
	mRecvBuf = (char*) malloc(mBufSize);
	mSock.openChild(fd);
	mSock.setOnListener([this](EdSmartSocket &sock, int event) {
		if(event == NETEV_READABLE) {
			ald("cnn readable, fd=%d", mSock.getFd());
			procRead();
		}
		else if(event == NETEV_WRITABLE) {
			ald("cnn writable, fd=%d", mSock.getFd());
			starttx();
		}
		else if(event == NETEV_DISCONNECTED) {
			ald("cnn disconnected, fd=%d", mSock.getFd());
//			if(mReqCtrl) {
//				endCtrl(mReqCtrl); mReqCtrl = nullptr;
//			}
			close();
			mpCtx->disconnectedCnn(this);
			mpCtx->freeCnn(mHandle);
		}
	});

	mCnnTimer.setOnListener([this](EdTimer &timer) {
		ali("*** service cnn timeout...");
		if(mTimerUpdate) {
			mTimerUpdate = 0;
			return;
		}
		ali("****   no received data for timer interval..., disconnect,...");
		timer.kill();
		close();
		mpCtx->freeCnn(mHandle);
	});
	mCnnTimer.set(CNN_TIMEOUT);

	return 0;
}

void ServCnn::close() {
#if 0
	if (mReqCtrl) {
		ali("close urlctrl forcely...");
		endCtrl(mReqCtrl);
		mReqCtrl = nullptr;
	}
#endif
	for(;mCtrlList.size()>0;) {
		auto pctrl = mCtrlList.front();
		endCtrl(pctrl);
	}
	mCnnTimer.kill();
	mSock.close();
	clearDummyCtrl();

	mLocalMsg.close();
}

void ServCnn::procRead() {
	auto rcnt = mSock.recvPacket(mRecvBuf, mBufSize);
	if (rcnt > 0) {
		mTimerUpdate = 1;
		CaHttpMsg msg;
		string msgdata;
		int fetch_res;
		if (mParsingStop == false) {
			auto wlen = mMsgFrame.feedPacket(mRecvBuf, rcnt);
			ald("feed consume bytes=%d, bufcnt=%d", wlen, rcnt);
			for (;;) {
				auto stt = mMsgFrame.status();
				ald("frame status=%d", (int )stt);
				if (stt == mMsgFrame.FS_HDR) {
					fetch_res = mMsgFrame.fetchMsg(msg);
					auto &url = msg.getUrlStr();
					CaHttpUrlParser urlparser;
					urlparser.parse(url);
					ald("msg dump=\n%s", msg.serialize());
					ald("url dump=%s", urlparser.dump());
					ald("request url path=%s", urlparser.path);
					clearDummyCtrl();
					auto pctrl = mpCtx->findAndAlloc(msg.getMethod(), urlparser.path);
					if (pctrl == nullptr) {
						pctrl = new CaHttp404UrlCtrl;
					}
					if (pctrl) {
						mCtrlList.emplace_back(pctrl);
						mReqCtrl = pctrl;
						mReqCtrl->procIncomingReqMsg(this, move(msg), fetch_res==CaHttpFrame::MSG_ONLY);
						if(fetch_res==CaHttpFrame::MSG_ONLY) mReqCtrl = nullptr;
					}
				} else if (stt == mMsgFrame.FS_DATA) {
					assert(mReqCtrl);
#if 1
					fetch_res = mMsgFrame.fetchData(msgdata);
					if(msgdata.size()>0) {
						auto dataend = (fetch_res == CaHttpFrame::MSG_DATA_END);
						mReqCtrl->procInReqData(move(msgdata), dataend);
						if(dataend) {
							mReqCtrl = nullptr;
						}
					} else {
						ald("data empty...");
						if(fetch_res == CaHttpFrame::MSG_DATA_END) {
							mReqCtrl->procInReqData(move(msgdata), true);
							mReqCtrl = nullptr;
						}
						break;
					}
#else
					auto &req = mReqCtrl->getReq();
					auto wrs = req.getReqWriteStream();
					fetch_res = mMsgFrame.fetchData(msgdata);
					if (wrs) {
						wrs->write(msgdata.data(), msgdata.size());
					} else {
						mReqCtrl->OnHttpReqData(move(msgdata));
					}
					if (fetch_res == CaHttpFrame::MSG_DATA_END) {
						mReqCtrl->OnHttpReqMsg();
						mReqCtrl = nullptr;
					}
#endif
				} else if (stt == mMsgFrame.FS_NONE) {
					break;
				}
			}
			//
			if ((ssize_t)wlen != rcnt) {
				mParsingStop = true;
				if (mReqCtrl) {
					mReqCtrl->OnHttpReqNonHttpData(mRecvBuf + wlen, rcnt - wlen);
				}
			}
		} else {
			if (mReqCtrl) {
				mReqCtrl->OnHttpReqNonHttpData(mRecvBuf, rcnt);
			}
		}
	}
}

void ServCnn::transfer(CaHttpUrlCtrl* pctrl) {
	starttx();
}

void ServCnn::starttx() {
	ald("starttx  start........");
	mTimerUpdate = 1;
	for (; mCtrlList.size() > 0;) {
		mRespCtrl = mCtrlList.front();
		ald("tx ctrl, handle=%d", mRespCtrl->getHandle());
		for (; mSock.isWritable();) {
			auto dp = mRespCtrl->getDataPtr();
			//		ali("get data, len=%ld, ptr=%0x", dp.second, (uint64_t)dp.first);
			if (dp.second > 0) {
				mPauseSend = false;
				auto ret = mSock.sendPacket(dp.first, dp.second);
				if (ret != SEND_FAIL) {
					mRespCtrl->consume(dp.second);
					if(ret != SEND_OK) {
						ald("*** tx pending,...");
						goto _TX_END;
					}
				} else {
					ale("### mRespCtrl: write fail...,fd=%d, send_size=%d", mSock.getFd(), dp.second);
					endCtrl(mRespCtrl);
					mRespCtrl = nullptr;
					goto _TX_END;
				}
			} else if (dp.second == 0) {
				ald("http response send complete");
				mPauseSend = false;
				endCtrl(mRespCtrl);
				mRespCtrl = nullptr;
			} else {
				// dp.second < 0
				ald("*** data lack..., pause status=%d", mPauseSend);
//				if(mPauseSend==true) {
//					mPauseSend = false;
//					ali("ctrl tx break...");
//					break;
//				}
				mPauseSend = true;
				auto lackret = mRespCtrl->procDataLack();
				if(!lackret) {
					endCtrl(mRespCtrl);
					mRespCtrl = nullptr;
				}
				if (mPauseSend)
					goto _TX_END;
			}

			if(mRespCtrl==nullptr) {
				ald("cur ctrl null, send stop,...");
				break;
			}
		}
		if(mRespCtrl) break; // if current url ctrl was not ended, exit.
	}
	_TX_END: ;
	ald("start tx end............................");
}

int ServCnn::writeData(CaHttpUrlCtrl& ctrl, const char* ubf, size_t len) {
	mPauseSend = false;
	mTimerUpdate = 1;
	auto *pctrl = mCtrlList.front();
	if(pctrl == &ctrl) {
		if(mSock.isWritable()) {
			auto wret = mSock.sendPacket(ubf, len);
			if(wret != SEND_OK) {
				ald("write pending...");
				mPauseSend = true;
			}
			return wret;
		}
	}
	return SEND_FAIL;
}

void ServCnn::gotoDummyCtrl(CaHttpUrlCtrl *pctrl) {
	ali("to dummy: ptr=%x", (long )pctrl);
	mDummyCtrlList.emplace_back(pctrl);
}

void ServCnn::endCtrl(CaHttpUrlCtrl *ctrl) {
	ald("end ctrl handle=%x", ctrl->getHandle());
	ctrl->OnHttpEnd();
	mCtrlList.pop_front();
	ald("   remove ctrl from list, size=%d", mCtrlList.size());
	mDummyCtrlList.emplace_back(ctrl);
	mRespCtrl = nullptr;
	ald("to dummy stack, handle=%x, dummy size=%d", ctrl->getHandle(), mDummyCtrlList.size());
}

void ServCnn::clearDummyCtrl() {
	if (mDummyCtrlList.size() > 0) {
		ald("clear dummy ctrl, size=%d", mDummyCtrlList.size());
		for (auto *pctrl : mDummyCtrlList) {
			ald("free dummy ctrl, ptr=%x", (long )pctrl);
			delete pctrl;
		}
		mDummyCtrlList.clear();
	}
}

void ServCnn::reserveWrite() {
	if(mSock.isWritable()) {
		mSock.reserveWrite();
	}
}


void ServCnn::postCloseConnection() {
	mLocalMsg.postMsg(LM_DISCONNECT, 0, 0);
}

void ServCnn::init_local_msgque() {
	auto handle = mLocalMsg.open([this](EdMsg &msg) {
		if(msg.msgid == LM_DISCONNECT) {
			ali("local msg: disconnect, ...");
			close();
		} else if(msg.msgid == LM_TEST) {
			ali("lm test...");
		}
		return 0;
	});
	ald("lm handle=%d", handle);
}
}
