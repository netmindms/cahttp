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
	BaseConnection();
	virtual ~BaseConnection();
	class CnnIf {
		virtual void OnRecvMsg(CaHttpMsg& msg)=0;
		virtual void OnRecvData(std::string &data)=0;
	};
	int connect(const std::string& ip, int port);
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
