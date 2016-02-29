/*
 * ReHttpSvrCtx.h
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REHTTPSVRCTX_H_
#define CAHTTP_REHTTPSVRCTX_H_

#include "BaseConnection.h"
#include "ReSvrCnn.h"
#include "ReUrlCtrl.h"

namespace cahttp {
class ReHttpServer;

class ReHttpSvrCtx {
	friend class ReHttpServer;
	friend class ReSvrCnn;
	friend class ReServTask;
public:
	ReHttpSvrCtx();
	virtual ~ReHttpSvrCtx();

private:
#if 0
	class localcnnif: public BaseConnection::CnnIf {
		friend class ReHttpSvrCtx;
		localcnnif(){
			pcnn=nullptr;
			psvrctx=nullptr;
		};
		virtual ~localcnnif(){};
		void set(ReHttpSvrCtx& svrctx, BaseConnection& cnn) {
			pcnn=&cnn;
			psvrctx=&svrctx;
		}
		virtual int OnWritable();
		virtual int OnCnn(int cnnstatus);

		BaseConnection *pcnn;
		ReHttpSvrCtx* psvrctx;
		std::list<ReUrlCtrl*> ctrls;
	};
	class recvif: public BaseConnection::RecvIf {
		friend class ReHttpSvrCtx;
		recvif(ReHttpSvrCtx& ctx, BaseConnection& cnn);
		virtual ~recvif();
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg) override ;
		virtual int OnData(std::string&& data) override;
		ReHttpSvrCtx& mSvrCtx;
		BaseConnection& mCnn;
	};

	struct cnnctx {
		uint32_t handle;
		BaseConnection cnn;
		localcnnif cnnif;
		std::list<ReUrlCtrl*> ctrls;
	};
	int procOnMsg(BaseConnection& cnn, upBaseMsg upmsg);
	int procOnData(std::string&& data);
#endif
	int newCnn(int fd);
	void init(ReHttpServer& svr);
	void dummyCnn(uint32_t handle);
	void clearCnnDummy();
	void close();
	ReHttpServer* getServer() {
		return mpSvr;
	}
	uint32_t mHandleSeed;
	std::unordered_map<uint32_t, ReSvrCnn> mCnns;
	std::list<uint32_t> mCnnDummy;
	ReHttpServer* mpSvr;
};

} /* namespace cahttp */

#endif /* CAHTTP_REHTTPSVRCTX_H_ */
