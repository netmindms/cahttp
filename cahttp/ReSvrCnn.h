/*
 * ReSvrCnn.h
 *
 *  Created on: Feb 17, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_RESVRCNN_H_
#define CAHTTP_RESVRCNN_H_
#include <ednio/EdEventFd.h>
#include <list>

#include "BaseConnection.h"
#include "BaseMsg.h"
namespace cahttp {

class ReHttpServer;
class ReUrlCtrl;
class ReHttpSvrCtx;

class ReSvrCnn {
	friend class ReHttpSvrCtx;
	friend class ReUrlCtrl;
public:
	ReSvrCnn();
	virtual ~ReSvrCnn();
	int init(uint32_t handle, uint fd, ReHttpSvrCtx& svr);
	void close();
private:
#if 0
	class ServRecvif: public BaseConnection::RecvIf {
		friend class ReSvrCnn;
		ServRecvif(ReSvrCnn& cnn);
		virtual ~ServRecvif();
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg) override;
		virtual int OnData(std::string&& data) override;
		ReSvrCnn& mCnn;
	};
	class ServCnnIf: public BaseConnection::CnnIf {
		friend class ReSvrCnn;
		ServCnnIf(ReSvrCnn& cnn);
		virtual ~ServCnnIf();
		virtual int OnWritable();
		virtual int OnCnn(int cnnstatus);
		ReSvrCnn& mCnn;
	};
#endif

	int procOnMsg();
	int procOnData();
	int procOnCnn(int cnnstatus);
	int procOnWritable();
	void reserveWrite() {
		mCnn->reserveWrite();
	}
	SEND_RESULT send(uint32_t handle, const char* ptr, size_t len);

	inline BaseConnection* getConnection() {
		return mCnn;
	}
	void dummyCtrl(uint32_t handle);
	void clearDummy();
	void endCtrl(uint32_t handle);
	void procDummyCtrls();

	std::list<ReUrlCtrl*> mCtrls;
	std::list<ReUrlCtrl*> mDummyCtrls;
	uint32_t mHandle;
	BaseConnection* mCnn;
	ReHttpServer* mSvr;
	ReHttpSvrCtx* mCtx;
#if 0
	ServRecvif mRecvIf;
	ServCnnIf mCnnIf;
#endif
	ReUrlCtrl* mpCurCtrl;
	edft::EdEventFd mEndEvt;
	uint32_t mCtrlHandleSeed;
	uint32_t mSendCtrlHandle;
	uint32_t mTxHandle;
	uint32_t mRxHandle;
};

} /* namespace cahttp */

#endif /* CAHTTP_RESVRCNN_H_ */
