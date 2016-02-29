/*
 * ReHttpServer.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include <string>
#include "GenericUrlCtrl.h"

#include "ReHttpSvrCtx.h"
#include "ReHttpServer.h"
#include "NotFoundUrl.h"
#include "flog.h"

using namespace std;

namespace cahttp {

ReHttpServer::ReHttpServer() {
	mpLocalCtx = nullptr;
	mIp = "";
	mPort = 0;
	mCfgCnnTimeout = 60;
	mJobOrder = 0;
	mTaskNum = 0;
}

ReHttpServer::~ReHttpServer() {
	close();
}

int ReHttpServer::start(int task_num) {
	if(task_num==0) {
		ali("server start in current thread.");
		mpLocalCtx = new ReHttpSvrCtx;
		mpLocalCtx->init(*this);
	} else {
		init_tasks(task_num);
	}
	mTaskNum = task_num;
	mLisSocket.setOnListener([this](int event) {
		if(event == edft::SOCK_EVENT_INCOMING_ACCEPT) {
			auto fd = mLisSocket.accept();
			ald("accet fd=%d", fd);
			if(mTaskNum > 0) {
				auto idx = (mJobOrder++)%mTaskNum;
				mTasks[idx]->postMsg(ReServTask::UM_NEWCNN, fd, 0);
			} else {
				if(fd>0) {
					auto r = mpLocalCtx->newCnn(fd);
					if(r) {
						mLisSocket.close();
					}
				} else {
					ale("### accept fail");
				}
			}
		}
	});
	if(mPort==0) {
		mPort = 7000;
	}
	if(mIp.empty()) {
		mIp = "0.0.0.0";
	}
	auto ret = mLisSocket.listenSock(mPort);
	ali("listen port=%d, ret=%d" , mPort, ret);
	return ret;
}

ReUrlCtrl* ReHttpServer::allocUrlCtrl(http_method method, const std::string& path) {
	ReUrlCtrl* res=nullptr;
	for(auto &m: mMethodMap) {
		if(m.first == method) {
			for(auto &r: m.second) {
				auto mr = r.rexp.matchParams(path);
				if(!mr.first) {
					res = r.alloc();
					res->setPathParams(move(mr.second));
					goto normal_exit;
				}
			}
		}
	}
	if(!res) {
		res = new NotFoundUrl;
	}
normal_exit:
	return res;
}

int ReHttpServer::setUrlReg(http_method method, const std::string& pattern, ReHttpServer::UrlAlloctor alloctor) {
	RegAllocList* al=nullptr;
	for(auto &m: mMethodMap) {
		if(m.first == method) {
			al = &m.second;
			break;
		}
	}
	if(!al) {
		mMethodMap.emplace_back();
		mMethodMap.back().first = method;
		al = &(mMethodMap.back().second);
	}
	al->emplace_back();
	auto &a = al->back();
	a.alloc = alloctor;
	return a.rexp.setPattern(pattern.data());
}

void ReHttpServer::close() {
	if(mLisSocket.getFd() >= 0) {
		mLisSocket.close();
		for (auto *t : mTasks) {
			t->postExit();
		}
		for (auto &t : mTasks) {
			t->wait();
			delete t;
		}
		mTasks.clear();
		if(mpLocalCtx) {
			mpLocalCtx->close();
			delete mpLocalCtx;
			mpLocalCtx = nullptr;
		}
	}
}

//
int ReHttpServer::setUrlRegLam(http_method method, const std::string& pattern, std::function<void()> lis) {
	setUrlReg(method, pattern, [lis](){
		return new GenericUrlCtrl;
	});
	return 0;
}

void ReHttpServer::init_tasks(int task_num) {
	mTaskNum = task_num;
	if(task_num > 0) {
		for (int i = 0; i < task_num; i++) {
			ReServTask *task = new ReServTask(this, i);
			mTasks.push_back(task);
			task->run();
		}
	}
	ali("start service tasks, task num=%d", mTasks.size());
}


void ReHttpServer::config(const char* param, const char* val) {
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

#ifdef UNIT_TEST
int cahttp::ReHttpServer::test() {
	class urlctrl: public ReUrlCtrl {
	public:
		urlctrl(int a, int b) {
			ale("a+b=%d", a+b);
		};
		virtual ~urlctrl(){};
	};

	ReHttpServer svr;
	svr.setUrlReg<ReUrlCtrl>(HTTP_GET, "/abc");
	svr.setUrlReg(HTTP_GET, "/([0-9]+)/abc", []()->ReUrlCtrl* {
		return new ReUrlCtrl;
	});


	{
		auto *pctrl = svr.allocUrlCtrl(HTTP_GET, "/asdfasf");
		assert(pctrl);
		auto &vs = pctrl->getPathParams();
		assert(vs.size()==0);
		delete pctrl;
	}

	{
		auto *pctrl = svr.allocUrlCtrl(HTTP_GET, "/12/abc");
		assert(pctrl);
		auto &vs = pctrl->getPathParams();
		assert(vs.size()==1);
		assert(vs[0]=="12");
		delete pctrl;
	}

	return 0;
}
#endif
} /* namespace cahttp */
