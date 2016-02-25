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

#include "CaHttpCommon.h"
#include "HttpMsgFrame2.h"

namespace cahttp {

class BaseConnection {
	friend class HttpReq;
	friend class ReHttpSvrCtx;
public:
	enum CH_E {
		CH_CONNECTED,
		CH_WRITABLE,
		CH_CLOSED,
		CH_MSG,
		CH_DATA,
	};
	typedef std::function<int(CH_E evt)> ChLis;
	struct _chlis {
		uint32_t handle;
		ChLis lis;
	};
#if 0
	class CnnIf {
	public:
		CnnIf(){};
		virtual ~CnnIf(){};

		// if return is not 0, read loop exit
		virtual int OnWritable()=0;
//		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg)=0;
//		virtual int OnData(std::string&& data)=0;
		virtual int OnCnn(int cnnstatus)=0;
	};
	class RecvIf {
	public:
		RecvIf(){};
		virtual ~RecvIf(){};
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg)=0;
		virtual int OnData(std::string&& data)=0;
	};
#endif
	typedef std::function<void (BaseMsg&, int)> MsgLis;
	BaseConnection();
	virtual ~BaseConnection();
	int openServer(int fd);
	virtual int connect(uint32_t ip, int port, int timeout=30000); // timeout 30 sec
#if 0
	void setRecvIf(RecvIf* pif) {
		mRecvIf = pif;
	}
	virtual uint32_t startSend(CnnIf* pif);
	virtual void changeSend(CnnIf* pif);
	virtual void endSend(uint32_t handle);
#endif
	virtual cahttp::SEND_RESULT send(uint32_t handle, const char* buf, size_t len);
	virtual void reserveWrite();
	virtual void close();
	inline bool isWritable() {
		return mSocket.isWritable();
	}
//	void setCallback(CnnIf* pif);
//	virtual void OnRecvMsg(CaHttpMsg &msg);
//	virtual void OnRecvData(std::string& data);
	uint32_t openTxCh(ChLis lis);
	uint32_t openRxCh(ChLis lis, bool front=false);
	void endTxCh(uint32_t h);
	void endRxCh(uint32_t h);
	BaseMsg* fetchMsg() {
		return mRecvMsg.release();
	};
	std::string fetchData() {
		return move(mRecvData);
	}
private:
	HttpMsgFrame2 mMsgFrame;
	edft::EdSmartSocket mSocket;
#if 0
	CnnIf *mNotiIf;
	RecvIf *mRecvIf;
#endif
	size_t mBufSize;
	char* mBuf;
	uint8_t mStatusFlag;
	std::list<_chlis> mTxChList;
	std::list<_chlis> mRxChList;
	uint32_t mHandleSeed;
	upBaseMsg mRecvMsg;
	std::string mRecvData;
	edft::EdTimer mCnnTimer;

	int procRead();
	void init_sock(bool svr, int fd);
	int procWritable();
	int procClosed();

};

} /* namespace cahttp */

#endif /* CAHTTP_BASECONNECTION_H_ */
