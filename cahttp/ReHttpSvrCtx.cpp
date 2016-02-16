/*
 * ReHttpSvrCtx.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG

#include "ReHttpSvrCtx.h"

#include "CaHttpUrlParser.h"
#include "flog.h"

namespace cahttp {

ReHttpSvrCtx::ReHttpSvrCtx() {
	mHandleSeed = 0;
}

ReHttpSvrCtx::~ReHttpSvrCtx() {
	// TODO Auto-generated destructor stub
}


int cahttp::ReHttpSvrCtx::localcnnif::OnWritable() {

}

int cahttp::ReHttpSvrCtx::localcnnif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	return psvrctx->procOnMsg(*pcnn, move(upmsg));
}

int cahttp::ReHttpSvrCtx::localcnnif::OnData(std::string&& data) {
	return psvrctx->procOnData(move(data));
}

int cahttp::ReHttpSvrCtx::localcnnif::OnCnn(int cnnstatus) {

}

void ReHttpSvrCtx::newCnn(int fd) {
	if(++mHandleSeed==0) ++mHandleSeed;
	auto &cnnctx = mCnns[mHandleSeed];
	cnnctx.handle = mHandleSeed;
	cnnctx.cnnif.set(*this, cnnctx.cnn);
	cnnctx.cnn.open(fd);

}

int ReHttpSvrCtx::procOnMsg(BaseConnection& cnn, upBaseMsg upmsg) {
	auto &u = upmsg->getUrl();
	CaHttpUrlParser parser;
	if(parser.parse(u)) {
		auto *pctrl = mSvr.allocUrlCtrl(upmsg->getMethod(), parser.path);
		if(pctrl) {
			auto *pmsg = upmsg.get();
			pctrl->init(move(upmsg));
			pctrl->OnMsgHdr(*pmsg);
			if(pmsg->getContentLenInt()==0) {
				pctrl->OnEnd();
			}
		}
		return 0;
	} else {
		ale("### url parsing error");
		return -1;
	}

}

int ReHttpSvrCtx::procOnData(std::string&& data) {

}

} /* namespace cahttp */
