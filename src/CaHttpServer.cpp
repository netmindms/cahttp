/*
 * CaHttpServer.cpp
 *
 *  Created on: Apr 24, 2015
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include <assert.h>
#include "flog.h"
#include "CaHttpServer.h"
#include "HttpServCnnCtx.h"
namespace cahttp {
#define CHECK_CALLER_TASK() (assert(mLisSock.getTask()==EdTask::getCurrentTask()))

CaHttpServer::CaHttpServer() {
	mpCnnCtx = nullptr;
	mIp = "";
	mPort = 0;
	mCfgCnnTimeout = 60;
	mJobOrder = 0;
	mTaskNum = 0;
	mLis = nullptr;
}

CaHttpServer::~CaHttpServer() {
	close();
}

void CaHttpServer::setTaskNum(int num) {
	mTaskNum = num;

}

int CaHttpServer::start(int task_num) {
	if(task_num == 0) {
		mpCnnCtx = new HttpServCnnCtx;
		mpCnnCtx->init(*this, 0);
	} else {
		init_tasks(task_num);
	}
	mTaskNum = task_num;
	mLisSock.setOnListener([this](EdSocket &sock, int event) {
		if(event == SOCK_EVENT_INCOMING_ACCEPT) {
			auto fd = sock.accept();
			if(mTaskNum>0) {
				auto idx = (mJobOrder++)%mTaskNum;
				mTasks[idx]->postMsg(ServTask::UM_NEWCNN, fd, 0);
			} else {
				mpCnnCtx->newCnn(fd);
			}
		}
	});
	if(mPort==0) {
		mPort = 8080;
	}
	if(mIp.empty()) {
		mIp = "0.0.0.0";
	}
	auto ret = mLisSock.listenSock(mPort, mIp.data());
	ali("listen port=%d", mPort);
	if(ret) {
		ale("### Error: listen fail, ret=%d", ret);
	}
	return ret;
}

void CaHttpServer::close() {
	if(mLisSock.getFd() >= 0) {
		CHECK_CALLER_TASK();
		mLisSock.close();
		for (auto *t : mTasks) {
			t->postExit();
		}
		for (auto &t : mTasks) {
			t->wait();
			delete t;
		}
		mTasks.clear();
		if(mpCnnCtx) {
			mpCnnCtx->close();
			delete mpCnnCtx; mpCnnCtx = nullptr;
		}
	}
}

void CaHttpServer::setOnListener(Lis lis) {
	mLis = lis;
}

void CaHttpServer::init_tasks(int task_num) {
	mTaskNum = task_num;
	if(task_num > 0) {
		for (int i = 0; i < task_num; i++) {
			ServTask *task = new ServTask(this);
			mTasks.push_back(task);
			task->run();
		}
	}
	ali("server task num=%d", mTasks.size());

}

const UrlMap* CaHttpServer::getUrlMap(http_method method) {
	UrlMap* pmap=nullptr;
	for(auto &mi: mMethodUrlVec) {
		if(mi.first == method) {
			pmap = &mi.second;
		}
	}
	return pmap;
}

UrlCtrlAlloc CaHttpServer::matchRegExUrl(smatch& result, const string& s) {
	bool ret;
	for(auto &rm : mUrlRegExMap) {
		ret = regex_match(s, result, rm.first);
		if(ret) {
			return rm.second;
		}
	}
	return nullptr;
}

void CaHttpServer::config(const char* param, const char* val) {
	if(!strcmp(param, "port")) {
		mPort = stoi(val);
	}
	else if(!strcmp(param, "ip")) {
		mIp = val;
	}
	else if(!strcmp(param, "connection-timeout")) {
		mCfgCnnTimeout = stoi(val);
	}
	else {
		assert(0);
		ale("### Error: invalid parameter=%s", param);
	}
}

void CaHttpServer::notifyNewConnection(ServCnn& cnn) {
	if(mLis) {
		mLis(SVRE_NEW_CONNECTION, cnn);
	}
}

void CaHttpServer::closeAllConnections() {
	ali("close all connections...")
	assert(mLisSock.getTask()==EdTask::getCurrentTask());
	if(mTaskNum > 0) {
		for(auto *task: mTasks) {
			task->sendMsg(ServTask::UM_CLOSE_ALLCNN);
		}
	} else {
		mpCnnCtx->close();
	}
}

void CaHttpServer::notifyCloseConnection(ServCnn& cnn) {
	if(mLis) {
		mLis(SVRE_CLOSE_CONNECTION, cnn);
	}
}

void CaHttpServer::dump() {
	ald("http server fd=%d, task=%0x, ptr=%0x", mLisSock.getFd(), (u64)mLisSock.getTask(), (u64)this);
}
}
