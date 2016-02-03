
/*
 * CaHttpCnn.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_INFO
#include "flog.h"
#include "CaHttpCommon.h"
#include "HttpCnn.h"
#include "CaHttpUrlParser.h"

using namespace std;

namespace cahttp {
HttpCnn::HttpCnn() {
	ald("default constructor...");
	mIsConnected = false;
	mIp = 0;
	mPort = 0;
	mError = 0;
	mCurMsgInfo.msgStrm = nullptr;
	mFrame.init();
}

HttpCnn::~HttpCnn() {
}

void HttpCnn::initSock() {
	mSock.setOnListener([this](EdSmartSocket &sock, int event) {
		if(event==NETEV_CONNECTED) {
			ald("connected...");
			mIsConnected = true;
			starttx();
		}
		else if(event == NETEV_READABLE) {
			procRead();
		}
		else if(event == NETEV_DISCONNECTED) {
			ali("disconnected...");
			sock.close();
			if(mIsConnected==false) {
				mError = -100;
			}
			procDisconnected();
		}
		else if(event == NETEV_WRITABLE) {
			ald("socket writable...");
			starttx();
		}
	});

}

void HttpCnn::close() {
	mSock.close();
}

void HttpCnn::setHostIpAddr(uint32_t ip, int port) {
	mIp = ip;
	mPort = port;
}

void HttpCnn::setHostIpAddr(const string& ipaddr, int port) {
	in_addr in;
	auto ret = inet_aton(ipaddr.data(), &in);
	if (ret) {
		setHostIpAddr(in.s_addr, port);
	}
}


int HttpCnn::getError() {
	return mError;
}

int HttpCnn::transfer() {
	assert(mCurMsgInfo.lis);assert(mCurMsgInfo.dataLis);
	int ret;
	if (mIsConnected == false) {
		initSock();
		ret = mSock.connect(mIp, mPort);
		ret = SEND_PENDING;
	} else {
		starttx();
	}
	return ret;
}

void HttpCnn::starttx() {
	ald("start tx, pipe size=%d", mPipeList.size());
	int ret;
	msg_pipe_t *ptxmsg=nullptr;
	if(mCurMsgInfo.msgStrm) ptxmsg = &mCurMsgInfo;
	for(;;) {
		if(!ptxmsg) {
			if(mSendQueList.size()>0) {
				ali("take send msg from send que, que_size=%d", mSendQueList.size());
				ptxmsg = mSendQueList.back();
				mSendQueList.pop_front();
			} else {
				ald("no send msg que, stop send,...");
//				assert(0);
				break;
			}
		}
		for (; mSock.isWritable();) {
			auto dp = ptxmsg->msgStrm->getDataPtr();
			if (dp.second > 0 && dp.first) {
				ret = mSock.sendPacket(dp.first, dp.second);
				if (ret == SEND_OK) {
					ptxmsg->msgStrm->consume(dp.second);
				} else if (ret == SEND_PENDING) {
					ptxmsg->msgStrm->consume(dp.second);
					break;
				} else if (ret == SEND_FAIL) {
					ale("### Error: write fail...");
					ptxmsg->lis(CE_FAIL);
					break;
				}
			} else if (dp.second == 0) {
				//			mLis(CE_SEND_COMPLETE);
				ptxmsg = nullptr;
				break;
			} else if (dp.second < 0 || dp.first==nullptr) {
				ald("tx underrun...");
				int di = ptxmsg->lis(CE_UNDERRUN);
				if (!di) {
					ald("  tx pause,...");
					break;
				} else {
					ald("  tx continue, ...");
				}
			}
		}

		if(ptxmsg) { // if current msg stream is not complete, exit loop
			break;
		}
	}
}

void HttpCnn::release(bool force) {
	if (force == false) {
		mCnnTimer.setOnListener([this](EdTimer &timer) {
			ali("conection idle timeout... connection free");
			mCnnTimer.kill();
			close();
		});
		mCnnTimer.set(1000);
	} else {
		close();
	}
}

void HttpCnn::procRead() {
#if 0
	msg_pipe_t* pmsg= nullptr;
	if(mCurMsgInfo.msgStrm==nullptr) {
		assert(mPipeList.size()>0);
		pmsg = &(mPipeList.front());
		mPipeList.pop_front();
	} else {
		pmsg = &mCurMsgInfo;
	}
#endif
	char buf[2048];
	int rcnt = mSock.recvPacket(buf, 2048);
	if (rcnt > 0) {
		// debug
		buf[rcnt]=0;
		ald("recv socket data:\n%s\n", buf);
		mFrame.feedPacket(buf, rcnt);
		int fs;
		for (;;) {
			fs = mFrame.status();
			ald("fs: frame status=%d", fs);
			if (fs == mFrame.FS_HDR) {
				ald("fs: header received");
				assert(mCurMsgInfo.msgStrm);
				auto res = mFrame.fetchMsg(mRecvMsg);
				mCurMsgInfo.lis(CE_RECV_MSGHDR);
				if (res == mFrame.MSG_ONLY) {
					mCurMsgInfo.lis(CE_RECV_MSG);
					if(mPipeList.size()>0) {
						ali("take msg from pipe, cur pipe size=%d", mPipeList.size());
						auto &pimsg = mPipeList.front();
						mCurMsgInfo = pimsg;
						mPipeList.pop_front();
					} else {
						ali("no pipe msg, ...");
						mCurMsgInfo.msgStrm = nullptr;
						mCurMsgInfo.lis = nullptr;
						mCurMsgInfo.dataLis = nullptr;
						break;
					}
				}
			} else if (fs == mFrame.FS_DATA) {
				ald("fs: data received...");
				mRespBuf.clear();
				auto res = mFrame.fetchData(mRespBuf);
				ald("fetch data, size=%d, result=%d", mRespBuf.size(), res);
				if (1) {//!mRespBuf.empty()) {
					mCurMsgInfo.dataLis(move(mRespBuf), res==mFrame.MSG_DATA_END);
					if(res == mFrame.MSG_DATA_END) {
						mCurMsgInfo.lis(CE_RECV_MSG);
						if(mPipeList.size()>0) {
							ali("take msg from pipe, cur pipe size=%d", mPipeList.size());
							auto &pimsg = mPipeList.front();
							mCurMsgInfo = pimsg;
							mPipeList.pop_front();
						} else {
							ald("no pipe msg, ...");
							mCurMsgInfo.msgStrm = nullptr;
							mCurMsgInfo.lis = nullptr;
							mCurMsgInfo.dataLis = nullptr;
							break;
						}
					} else if(res == mFrame.MSG_DATA_EMPTY) {
						break;
					}
				} else {
//					if(res == mFrame.MSG_DATA_END) mLis(CE_RESP_END);
					break;
				}
			} else if (fs == mFrame.FS_CONT) {
				ald("fs: continue...");
				break;
			}
//			else if(fs==mFrame.FS_END) {
//				ald("fs: end");
//				mReq->acceptRecvEnd();
//				break;
//			}
			else if (fs == mFrame.FS_NONE) {
				break;
			} else {
				assert(0);
			}
		}
	}

}

CaHttpMsg&& HttpCnn::fetchRecvMsg() {
	return move(mRecvMsg);
}


int HttpCnn::writeData(const char* ptr, size_t len) {
	return mSock.sendPacket(ptr, len);
}

void HttpCnn::addMsgStream(HttpMsgStream *pstrm, Lis lis, DataLis dlis) {
	if(!mCurMsgInfo.msgStrm) {
		mCurMsgInfo.msgStrm = pstrm;
		mCurMsgInfo.lis = lis;
		mCurMsgInfo.dataLis = dlis;
	} else {
		// add
		mPipeList.emplace_back();
		auto &pm = mPipeList.back();
		pm.msgStrm = pstrm;
		pm.lis = lis;
		pm.dataLis = dlis;
		mSendQueList.push_back(&pm);
		ali("add msg stream to pipe, size=%d", mPipeList.size());
	}
}

void HttpCnn::procDisconnected() {
	if(mCurMsgInfo.msgStrm) {
		mCurMsgInfo.lis(CE_CLOSED);
	}

	for(auto &pi: mPipeList) {
		pi.lis(CE_CLOSED);
	}
}
}
