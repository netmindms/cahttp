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
	mpSvr = nullptr;
}

ReHttpSvrCtx::~ReHttpSvrCtx() {
	// TODO Auto-generated destructor stub
}


void ReHttpSvrCtx::newCnn(int fd) {
	if(++mHandleSeed==0) ++mHandleSeed;
	auto &scnn = mCnns[mHandleSeed];
	ali("new connection, handle=%d, cnn_count=%d", mHandleSeed, mCnns.size());
	scnn.init(mHandleSeed, fd, *mpSvr);
}

void ReHttpSvrCtx::init(ReHttpServer& svr) {
	mpSvr = &svr;
}


#if 0


int cahttp::ReHttpSvrCtx::localcnnif::OnWritable() {

}

int cahttp::ReHttpSvrCtx::localcnnif::OnCnn(int cnnstatus) {

}
int ReHttpSvrCtx::procOnMsg(BaseConnection& cnn, upBaseMsg upmsg) {
	auto &u = upmsg->getUrl();
	CaHttpUrlParser parser;
	if(parser.parse(u)) {
		auto *pctrl = mSvr.allocUrlCtrl(upmsg->getMethod(), parser.path);
		if(pctrl) {
			cnn.changeSend(pctrl->getCnnIf());
			auto *pmsg = upmsg.get();
			pctrl->init(move(upmsg), cnn);
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


ReHttpSvrCtx::recvif::recvif(ReHttpSvrCtx& ctx, BaseConnection& cnn): mSvrCtx(ctx), mCnn(cnn) {
}

ReHttpSvrCtx::recvif::~recvif() {
}

int ReHttpSvrCtx::recvif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	mSvrCtx.procOnMsg(mCnn, move(upmsg));
	return 0;
}

int ReHttpSvrCtx::recvif::OnData(std::string&& data) {
	return 0;
}
#endif


} /* namespace cahttp */
