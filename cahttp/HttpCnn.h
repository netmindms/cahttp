/*
 * CaHttpCnn.h
 *
 *  Created on: Apr 16, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPCNN_H_
#define SRC_HTTPCNN_H_

#include <stdio.h>
#include <functional>
#include <ednio/EdNio.h>
#include "CaHttpFrame.h"
#include "CaHttpReq.h"
#include "CaHttpCommon.h"
#include "HttpMsgStream.h"

using namespace edft;
namespace cahttp {
class CaHttpReq;

class HttpCnn
{
	friend class CaHttpReq;

public:
	enum CE { CE_FAIL=-1, CE_SEND_COMPLETE, CE_CLOSED, CE_UNDERRUN, CE_RECV_MSGHDR, CE_RECV_DATA, CE_RECV_MSG, CE_RECV_END} ;
	typedef std::function<int(CE)> Lis;
	typedef std::function<void(string&& data, int data_status)> DataLis;
	HttpCnn();
	virtual ~HttpCnn();
	void setHostIpAddr(const std::string &ipaddr, int port=80);
	void setHostIpAddr(uint32_t ip, int port=80);
	void close();
	void release(bool force=false);
	int getError();

	void addMsgStream(HttpMsgStream *pstrm, Lis lis, DataLis dlis);

	CaHttpMsg& getRecvMsg() {
		return mRecvMsg;
	}
private:
	struct msg_pipe_t {
		HttpMsgStream* msgStrm;
		Lis lis;
		DataLis dataLis;
	};
	EdSmartSocket mSock;
	bool mIsConnected;
	msg_pipe_t mCurMsgInfo;
	unsigned int mIp;
	unsigned short mPort;
	CaHttpFrame mFrame;
	int mError;
	EdTimer mCnnTimer;
	string mRespBuf;
	CaHttpMsg mRecvMsg;
	std::list<msg_pipe_t> mPipeList;
	std::list<msg_pipe_t*> mSendQueList;
	void initSock();
	int txMsg();
	void starttx();
	int transfer();
	void procRead();
	void procDisconnected();
	CaHttpMsg &&fetchRecvMsg();
	int writeData(const char* ptr, size_t len);
};
}
#endif /* SRC_HTTPCNN_H_ */
