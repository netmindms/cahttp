/*
* ReSvrCnn.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_VERBOSE

#include "CaHttpUrlParser.h"
#include "ReHttpServer.h"
#include "ReSvrCnn.h"
#include "ReHttpSvrCtx.h"

#include "flog.h"
namespace cahttp {

ReSvrCnn::ReSvrCnn() /*: mRecvIf(*this), mCnnIf(*this) */ {
	mHandle = 0;
	mSvr = nullptr;
	mCnn = nullptr;
	mpCurCtrl = nullptr;
	mCtx = nullptr;
	mCtrlHandleSeed=0;
	mSendCtrlHandle=0;
}

ReSvrCnn::~ReSvrCnn() {
	close();
}

int ReSvrCnn::init(uint32_t handle, uint fd, ReHttpSvrCtx& ctx) {
	mCtx = &ctx;
	mEndEvt.setOnListener([this](edft::EdEventFd& efd, int cnt) {
		ald("ctrl end event, finished ctrls=%d", mDummyCtrls.size());
		procDummyCtrls();
	});
	auto efd = mEndEvt.open();
	alv("end event fd=%d", efd);
	if(fd<0) {
		ale("### Error: cnn event object open fail, fd=%d", fd );
		return -1;
	}

//	mHandle = handle;
	mSvr = mCtx->getServer();
	mCnn = new BaseConnection;
#if 1
	mRxHandle = mCnn->openRxCh([this](BaseConnection::CH_E evt) {
		if(evt == BaseConnection::CH_E::CH_MSG) {
			return procOnMsg();
		} else if(evt == BaseConnection::CH_E::CH_DATA) {
			return procOnData();
		} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
			return procOnCnn(0);
		}
		assert(0);
		return 1;
	});
	mTxHandle = mCnn->openTxCh([this](BaseConnection::CH_E evt) {
		if(evt == BaseConnection::CH_E::CH_WRITABLE) {
			return procOnWritable();
		} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
			return procOnCnn(0);
		}
		assert(0);
		return 1;
	});
#else
	mCnn->setRecvIf(&mRecvIf);
	mCnn->startSend(&mCnnIf);
#endif
	mCnn->openServer(fd);
	return 0;
}

int ReSvrCnn::procOnMsg() {
	alv("proc on msg");
	upBaseMsg upmsg(mCnn->fetchMsg());
	auto &u = upmsg->getUrl();
	CaHttpUrlParser parser;
	if(!parser.parse(u)) {
		auto *pctrl = mSvr->allocUrlCtrl(upmsg->getMethod(), parser.path);
		if(pctrl) {
			if(++mCtrlHandleSeed==0) mCtrlHandleSeed++;
			mpCurCtrl = pctrl;
			mCtrls.push_back(pctrl);
//			auto hsend = cnn.startSend(pctrl->getCnnIf());
			ald("new url handle=%d, ctrl cnt=%d", mCtrlHandleSeed, mCtrls.size());
			auto *pmsg = upmsg.get();
			pctrl->init(move(upmsg), *this, mCtrlHandleSeed);
			if(mSendCtrlHandle==0) mSendCtrlHandle = mCtrlHandleSeed;
			pctrl->OnHttpReqMsgHdr(*pmsg);
		}
		return 0;
	} else {
		ale("### url parsing error");
		close();
		return -1;
	}

}



void ReSvrCnn::dummyCtrl(uint32_t handle) {
	ali("goto dummy list, handle=%d, ctrls_cnt=%d, dummy_cnt=%d", handle, mCtrls.size(), mDummyCtrls.size());
	assert(handle==mCtrls.front()->getHandle());
	mDummyCtrls.splice(mDummyCtrls.end(), mCtrls, mCtrls.begin());
	ali("  dummy list cnt=%d", mDummyCtrls.size());
}

int ReSvrCnn::procOnData() {
	assert(mpCurCtrl);
	return mpCurCtrl->procOnData(mCnn->fetchData());
}

void ReSvrCnn::clearDummy() {
	for(auto *p: mDummyCtrls) {
		delete p;
	}
	mDummyCtrls.clear();
}

#if 0
ReSvrCnn::ServRecvif::ServRecvif(ReSvrCnn& cnn): mCnn(cnn) {
}

ReSvrCnn::ServRecvif::~ServRecvif() {
}

int cahttp::ReSvrCnn::ServRecvif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
	return mCnn.procOnMsg(move(upmsg));
}



int cahttp::ReSvrCnn::ServRecvif::OnData(std::string&& data) {
	mCnn.procOnData(move(data));
	return 0;
}
#endif

void ReSvrCnn::close() {
	if(mHandle) {
		assert(mCnn);
		mCnn->close();
		delete mCnn; mCnn=nullptr;
		mEndEvt.close();
		mCtx->dummyCnn(mHandle);
		mHandle=0;
	}
}

int ReSvrCnn::procOnCnn(int cnnstatus) {
	if(!cnnstatus) {
		ali("serv cnn disconnected, handle=%d", mHandle);
		if(mCtrls.size()) {
			ali("prematurly finished ctrl cnt=%d", mCtrls.size());
			mDummyCtrls.splice(mDummyCtrls.end(), mCtrls);
		}
		procDummyCtrls();
		close();
	}
	return 0;
}

int ReSvrCnn::procOnWritable() {
	for(auto &pctrl: mCtrls) {
		pctrl->procOnWritable();
		if(pctrl->isComplete()==false) {
			alv("ctrl not complete, stop write");
			break;
		}
	}
	return 0;
}

SEND_RESULT ReSvrCnn::send(uint32_t handle, const char* ptr, size_t len) {
	if(mSendCtrlHandle == handle) {
		return mCnn->send(1, ptr, len);
	} else {
		return SEND_RESULT::SEND_NEXT;
	}
}

void ReSvrCnn::endCtrl(uint32_t handle) {
	assert(mCtrls.size()>0);
	assert(mCtrls.front()->getHandle()==handle);
	mDummyCtrls.splice(mDummyCtrls.end(), mCtrls, mCtrls.begin());
	mEndEvt.raise();
	if(mCtrls.size()>0) {
		mSendCtrlHandle = mCtrls.front()->getHandle();
	} else {
		mSendCtrlHandle = 0;
	}
}

#if 0
ReSvrCnn::ServCnnIf::ServCnnIf(ReSvrCnn& cnn): mCnn(cnn) {
}

ReSvrCnn::ServCnnIf::~ServCnnIf() {
}

int ReSvrCnn::ServCnnIf::OnWritable() {
	return mCnn.procOnWritable();
}

int ReSvrCnn::ServCnnIf::OnCnn(int cnnstatus) {
	return mCnn.procOnCnn(cnnstatus);
}
#endif

void ReSvrCnn::procDummyCtrls() {
	for(;mDummyCtrls.size()>0;) {
		auto &pctrl = mDummyCtrls.front();
		ald("  ctrl comp, handle=%d", pctrl->getHandle());
		pctrl->OnHttpEnd();
		delete pctrl;
		mDummyCtrls.pop_front();
	}
}

} /* namespace cahttp */
