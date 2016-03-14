/*
 * BaseCnn.h
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_BASECNN_H_
#define CAHTTP_BASECNN_H_

#include <string>
#include <ednio/EdNio.h>
#include <ednio/EdLocalEvent.h>

#include "CaHttpCommon.h"
#include "HttpMsgFrame2.h"

namespace cahttp {

class BaseCnn {
	friend class HttpReq;
	friend class ReHttpSvrCtx;
	friend class ReSvrCnn;
private:
	union status_t {
			unsigned char val;
			struct {
				uint8_t connected:1;
				uint8_t server:1;
			};
			struct {
				uint8_t cache:2;
			};
		};
public:
	enum CH_E {
		CH_CONNECTED,
		CH_WRITABLE,
		CH_CLOSED,
		CH_MSG,
		CH_DATA,
	};
	enum CFG {
		eIdleTimer=0,
	};
	typedef std::function<void (BaseMsg&, int)> MsgLis;
	BaseCnn();
	virtual ~BaseCnn();
	void setOnListener(std::function<void(CH_E)> lis) {
		mLis = lis;
	}
	int openServer(int fd);
	virtual int connect(uint32_t ip, int port, int timeout, std::function<void(CH_E)> lis); // timeout 30 sec
	virtual cahttp::SR send(const char* buf, size_t len);
	virtual void sendEnd();
	virtual void recvEnd();
	virtual void close();
	virtual void reserveWrite();
	inline bool isWritable() {
		return mSocket.isWritable();
	}
	virtual void OnDisconnected();
	virtual void OnIdle();

	BaseMsg* fetchMsg() {
		return mRecvMsg.release();
	};
	std::string fetchData() {
		std::string s;
		s = move(mRecvData); mRecvData.clear();
		return move(s);
	}
	uint32_t ip() {
		return mSvrIp;
	}
	uint16_t port() {
		return mSvrPort;
	}
	void config(CFG cfg, int val) {
		switch(cfg) {
		case eIdleTimer:

			break;
		}
	};
	std::pair<uint32_t, uint16_t> getRmtAddr() {
		return {mSvrIp, mSvrPort};
	}

private:
	status_t mStatus;
	uint32_t mSvrIp;
	uint16_t mSvrPort;
	HttpMsgFrame2 mMsgFrame;
	std::function<void(CH_E)> mLis;
	size_t mBufSize;
	char* mBuf;
	uint8_t mStatusFlag;
	uint32_t mHandleSeed;
	upBaseMsg mRecvMsg;
	std::string mRecvData;
	edft::EdTimer mCnnTimer;
	edft::EdLocalEvent mLocalEvent;


	int procRead();
	void init_sock(bool svr, int fd);
	int procWritable();
	int procClosed();
	void forceCloseChannel(uint32_t rx, uint32_t tx);
	void startIdleTimer();
protected:
	edft::EdSmartSocket mSocket;
};

} /* namespace cahttp */

#endif /* CAHTTP_BASECNN_H_ */
