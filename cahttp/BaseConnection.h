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
	class CnnIf {
	public:
		CnnIf(){};
		virtual ~CnnIf(){};
		virtual void OnWritable()=0;
		virtual void OnMsg(std::unique_ptr<BaseMsg> upmsg)=0;
		virtual void OnData(std::string&& data)=0;
		virtual void OnCnn(int cnnstatus)=0;
	};
	typedef std::function<void (BaseMsg&, int)> MsgLis;
	BaseConnection();
	virtual ~BaseConnection();
	int connect(uint32_t ip, int port);
	uint32_t startSend(CnnIf* pif);
	void endSend(uint32_t handle);
	int send(const char* buf, size_t len);
	void reserveWrite();
	void close();
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
