/*
 * ReHttpSvrCtx.h
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REHTTPSVRCTX_H_
#define CAHTTP_REHTTPSVRCTX_H_

#include "BaseConnection.h"
#include "ReHttpServer.h"
#include "ReUrlCtrl.h"

namespace cahttp {

class ReHttpSvrCtx {
public:
	ReHttpSvrCtx();
	virtual ~ReHttpSvrCtx();

private:
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
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg);
		virtual int OnData(std::string&& data);
		virtual int OnCnn(int cnnstatus);

		BaseConnection *pcnn;
		ReHttpSvrCtx* psvrctx;
		std::list<ReUrlCtrl*> ctrls;
	};

	struct cnnctx {
		uint32_t handle;
		BaseConnection cnn;
		localcnnif cnnif;
		std::list<ReUrlCtrl*> ctrls;
	};

	void newCnn(int fd);
	int procOnMsg(BaseConnection& cnn, upBaseMsg upmsg);
	int procOnData(std::string&& data);

	uint32_t mHandleSeed;
	std::unordered_map<uint32_t, cnnctx> mCnns;
	ReHttpServer mSvr;
	std::list<upReUrlCtrl> mUrlDummy;
	std::list<uint32_t> mCnnDummy;
};

} /* namespace cahttp */

#endif /* CAHTTP_REHTTPSVRCTX_H_ */
