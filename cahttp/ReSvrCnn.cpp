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

ReSvrCnn::ReSvrCnn() {
	mHandle = 0;
	mSvr = nullptr;
	mCnn = nullptr;
	mpCurCtrl = nullptr;
	mCtx = nullptr;
	mHandleSeed=0;
	mSendCtrlHandle=0;
	mRxHandle=0;
	mTxHandle=0;
}

ReSvrCnn::~ReSvrCnn() {
	assert(mHandle==0);
	if(mCnn) {
		delete mCnn;
	}
}

int ReSvrCnn::init(uint32_t handle, uint fd, ReHttpSvrCtx& ctx) {
	mCtx = &ctx;
	mEndEvt.setOnListener([this](int cnt) {
		ald("ctrl end request event, finished ctrls=%d", mDummyCtrls.size());
		procDummyCtrls();
	});
	auto efd = mEndEvt.open();
	ald("ctrl end event obj fd=%d", efd);
	if(fd<0) {
		ale("### Error: cnn event object open fail, fd=%d", fd );
		return -1;
	}

	mHandle = handle;
	mSvr = mCtx->getServer();
	mCnn = new BaseConnection;
	mCnn->setDefRxListener([this](BaseConnection::CH_E evt) {
		if(evt == BaseConnection::CH_E::CH_MSG) {
			return procOnMsg();
		} else if(evt == BaseConnection::CH_E::CH_DATA) {
			ale("### error : unexpected event: channel data");
			assert(0);
			return 1;
		} else if(evt == BaseConnection::CH_E::CH_CLOSED) {
			ald("default rx chan closed");
			return procOnCnn(0);
		}
		assert(0);
		return 1;
	});
	mCnn->openServer(fd);
	return 0;
}

int ReSvrCnn::procOnMsg() {
	ald("new request message ");
	upBaseMsg upmsg(mCnn->fetchMsg());
	auto &u = upmsg->getUrl();
	CaHttpUrlParser parser;
	if(!parser.parse(u)) {
		ali("method=%s, url=%s", http_method_str(upmsg->getMethod()), u);
		auto *pctrl = mSvr->allocUrlCtrl(upmsg->getMethod(), parser.path);
		if(pctrl) {
			if(++mHandleSeed==0) mHandleSeed++;
			mpCurCtrl = pctrl;
			mCtrls.push_back(pctrl);
			ald("new url ctrl, handle=%d, ctrl cnt=%d", mHandleSeed, mCtrls.size());
			if(mSendCtrlHandle==0) mSendCtrlHandle = mHandleSeed;
			pctrl->init(move(upmsg), *this, mHandleSeed);
		} else {
			assert(0);
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

#if 0
int ReSvrCnn::procOnData() {
	assert(mpCurCtrl);
	return mpCurCtrl->procOnData(mCnn->fetchData());
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

#endif

void ReSvrCnn::clearDummy() {
	for(auto *p: mDummyCtrls) {
		delete p;
	}
	mDummyCtrls.clear();
}

void ReSvrCnn::close() {
	if(mHandle) {
		ald("close sever cnn, h=%d", mHandle);
		assert(mCnn);
		mCnn->close();
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


SR ReSvrCnn::send(uint32_t handle, const char* ptr, size_t len) {
	if(mSendCtrlHandle == handle) {
		return mCnn->send(1, ptr, len);
	} else {
		return SR::eNext;
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


void ReSvrCnn::procDummyCtrls() {
	for(;mDummyCtrls.size()>0;) {
		auto &pctrl = mDummyCtrls.front();
		ald("  ctrl comp, handle=%d", pctrl->getHandle());
		pctrl->OnHttpEnd(pctrl->getErr());
		delete pctrl;
		mDummyCtrls.pop_front();
	}
}

} /* namespace cahttp */
