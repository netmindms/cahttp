/*
 * BaseConnection.h
 *
 *  Created on: Jan 26, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_BASECONNECTION_H_
#define CAHTTP_BASECONNECTION_H_

#include <string>
#include <ednio/EdNio.h>
#include <ednio/EdLocalEvent.h>

#include "CaHttpCommon.h"
#include "HttpMsgFrame2.h"

namespace cahttp {

class BaseConnection {
	friend class HttpReq;
	friend class ReHttpSvrCtx;
	friend class ReSvrCnn;
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
	typedef std::function<int(CH_E evt)> ChLis;
	struct _chlis {
		uint32_t handle;
		ChLis lis;
	};

	typedef std::function<void (BaseMsg&, int)> MsgLis;
	BaseConnection();
	virtual ~BaseConnection();
	int openServer(int fd);
	virtual int connect(uint32_t ip, int port, int timeout=30000); // timeout 30 sec

	virtual cahttp::SR send(uint32_t handle, const char* buf, size_t len);
	virtual void reserveWrite();
	virtual void close();
	inline bool isWritable() {
		return mSocket.isWritable();
	}
	virtual void OnDisconnected();
	virtual void OnIdle();

	uint32_t openTxCh(ChLis lis);
	uint32_t openRxCh(ChLis lis);
	void endTxCh(uint32_t h);
	void endRxCh(uint32_t h);
	void removeRxChannel(uint32_t h);
	void removeTxChannel(uint32_t h);
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

	bool isIdle() {
		return (mTxChList.size() || mRxChList.size());
	}
	void forceCloseChannel(uint32_t rx, uint32_t tx);
	void setHandle(uint32_t handle) {
		mHandle = handle;
	}
	uint32_t getHandle() {
		return mHandle;
	}
	void setDefRxListener(ChLis lis) {
		mDefRxLis = lis;
	}

	size_t getRxChCount();
	size_t getTxChCount();
private:
	uint32_t mSvrIp;
	uint16_t mSvrPort;
	HttpMsgFrame2 mMsgFrame;

	size_t mBufSize;
	char* mBuf;
	uint8_t mStatusFlag;
	std::list<_chlis> mTxChList;
	std::list<_chlis> mRxChList;
	uint32_t mHandleSeed;
	upBaseMsg mRecvMsg;
	std::string mRecvData;
	edft::EdTimer mCnnTimer;
	ChLis mDefRxLis;
	std::list<_chlis> mDummyChannels;
	edft::EdLocalEvent mLocalEvent;
	uint32_t mHandle;

	int procRead();
	void init_sock(bool svr, int fd);
	int procWritable();
	int procClosed();

	void startIdleTimer();
protected:
	edft::EdSmartSocket mSocket;
};

} /* namespace cahttp */

#endif /* CAHTTP_BASECONNECTION_H_ */
