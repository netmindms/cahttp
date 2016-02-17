/*
* ReSvrCnn.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include "CaHttpUrlParser.h"
#include "ReHttpServer.h"
#include "ReSvrCnn.h"
#include "flog.h"
namespace cahttp {

ReSvrCnn::ReSvrCnn(): mRecvIf(*this) {
	mHandle = 0;
	mSvr = nullptr;
	mCnn = nullptr;
}

ReSvrCnn::~ReSvrCnn() {
}

int ReSvrCnn::init(uint32_t handle, uint fd, ReHttpServer& svr) {
	mHandle = handle;
	mSvr = &svr;
	mCnn = new BaseConnection;
	mCnn->setRecvIf(&mRecvIf);
	mCnn->openServer(fd);
	return 0;
}

int ReSvrCnn::procOnMsg(upBaseMsg upmsg) {
	BaseConnection& cnn = *mCnn;
	auto &u = upmsg->getUrl();
	CaHttpUrlParser parser;
	if(!parser.parse(u)) {
		auto *pctrl = mSvr->allocUrlCtrl(upmsg->getMethod(), parser.path);
		if(pctrl) {
			auto hsend = cnn.startSend(pctrl->getCnnIf());
			auto *pmsg = upmsg.get();
			pctrl->init(move(upmsg), cnn, hsend);
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


int cahttp::ReSvrCnn::recvif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	mCnn.procOnMsg(move(upmsg));
	return 0;
}

ReSvrCnn::recvif::recvif(ReSvrCnn& cnn): mCnn(cnn) {
}

ReSvrCnn::recvif::~recvif() {
}

int cahttp::ReSvrCnn::recvif::OnData(std::string&& data) {
	return 0;
}

} /* namespace cahttp */
