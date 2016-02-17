/*
 * ReSvrCnn.h
 *
 *  Created on: Feb 17, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_RESVRCNN_H_
#define CAHTTP_RESVRCNN_H_
#include <list>

#include "BaseConnection.h"
#include "BaseMsg.h"
namespace cahttp {

class ReHttpServer;
class ReUrlCtrl;

class ReSvrCnn {
	friend class ReHttpSvrCtx;
public:
	ReSvrCnn();
	virtual ~ReSvrCnn();
	int init(uint32_t handle, uint fd, ReHttpServer& svr);
private:
	class recvif: public BaseConnection::RecvIf {
		friend class ReSvrCnn;
		recvif(ReSvrCnn& cnn);
		virtual ~recvif();
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg) override;
		virtual int OnData(std::string&& data) override;
		ReSvrCnn& mCnn;
	};

	int procOnMsg(upBaseMsg upmsg);

	std::list<ReUrlCtrl*> mCtrls;
	uint32_t mHandle;
	BaseConnection* mCnn;
	ReHttpServer* mSvr;
	recvif mRecvIf;
};

} /* namespace cahttp */

#endif /* CAHTTP_RESVRCNN_H_ */
