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

#include "CaHttpMsg.h"
#include "HttpMsgFrame.h"

namespace cahttp {

class BaseConnection {
public:
	class CnnIf {
	public:
		CnnIf(){};
		virtual ~CnnIf(){};
		virtual void OnWritable()=0;
	};
	typedef std::function<void (CaHttpMsg&, int)> MsgLis;
	BaseConnection();
	virtual ~BaseConnection();
	int connect(uint32_t ip, int port);
	uint32_t startSend(CnnIf* pif);
	int send(const char* buf, size_t len);
//	void setCallback(CnnIf* pif);
//	virtual void OnRecvMsg(CaHttpMsg &msg);
//	virtual void OnRecvData(std::string& data);
private:
	HttpMsgFrame mMsgFrame;
	edft::EdSmartSocket mSocket;
	CnnIf *mNotiIf;
	size_t mBufSize;
	char* mBuf;

	int procRead();
};

} /* namespace cahttp */

#endif /* CAHTTP_BASECONNECTION_H_ */
