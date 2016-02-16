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

#include "HttpMsgFrame2.h"

namespace cahttp {

class BaseConnection {
public:
	enum SEND_RESULT {
		SEND_PENDING=-1,
		SEND_OK=0,
		SEND_FAIL=1,
		SEND_NEXT=2,
	};
	class CnnIf {
	public:
		CnnIf(){};
		virtual ~CnnIf(){};

		// if return is not 0, read loop exit
		virtual int OnWritable()=0;
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg)=0;
		virtual int OnData(std::string&& data)=0;
		virtual int OnCnn(int cnnstatus)=0;
	};
	typedef std::function<void (BaseMsg&, int)> MsgLis;
	BaseConnection();
	virtual ~BaseConnection();
	int openServer(int fd);
	virtual int connect(uint32_t ip, int port);
	virtual uint32_t startSend(CnnIf* pif);
	virtual void changeSend(CnnIf* pif);
	virtual void endSend(uint32_t handle);
	virtual int send(uint32_t handle, const char* buf, size_t len);
	virtual void reserveWrite();
	virtual void close();
	inline bool isWritable() {
		return mSocket.isWritable();
	}
//	void setCallback(CnnIf* pif);
//	virtual void OnRecvMsg(CaHttpMsg &msg);
//	virtual void OnRecvData(std::string& data);
private:
	HttpMsgFrame2 mMsgFrame;
	edft::EdSmartSocket mSocket;
	CnnIf *mNotiIf;
	size_t mBufSize;
	char* mBuf;
	uint8_t mStatusFlag;

	int procRead();
};

} /* namespace cahttp */

#endif /* CAHTTP_BASECONNECTION_H_ */
