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
	typedef std::function<void (CaHttpMsg&, int)> MsgLis;
	BaseConnection();
	virtual ~BaseConnection();
	int connect(const std::string& ip, int port);
//	void setCallback(CnnIf* pif);
//	virtual void OnRecvMsg(CaHttpMsg &msg);
//	virtual void OnRecvData(std::string& data);
private:
	HttpMsgFrame mMsgFrame;
	edft::EdSmartSocket mSocket;
//	CnnIf *mNotiIf;
	size_t mBufSize;
	char* mBuf;

	int procRead();
};

} /* namespace cahttp */

#endif /* CAHTTP_BASECONNECTION_H_ */
