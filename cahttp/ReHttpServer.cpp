/*
 * ReHttpServer.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG
#include "GenericUrlCtrl.h"

#include "ReHttpSvrCtx.h"
#include "ReHttpServer.h"
#include "NotFoundUrl.h"
#include "flog.h"

namespace cahttp {

ReHttpServer::ReHttpServer() {
	mpLocalCtx = nullptr;
}

ReHttpServer::~ReHttpServer() {
	close();
}

int ReHttpServer::start(int tasknum) {
	if(tasknum==0) {
		mpLocalCtx = new ReHttpSvrCtx;
		mpLocalCtx->init(*this);
	} else {

	}

	mSocket.setOnListener([this](edft::EdSocket& sock, int event) {
		if(event == edft::SOCK_EVENT_INCOMING_ACCEPT) {
			if(mpLocalCtx) {
				auto fd = sock.accept();
				ald("accet fd=%d", fd);
				if(fd>0) {
					auto r = mpLocalCtx->newCnn(fd);
					if(r) {
						sock.close();
					}
				} else {
					ale("### accept fail");
				}
			}
		}
	});
	auto fd=mSocket.listenSock(9000);
	return fd>=0 ? 0: -1;
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
	mSocket.close();
	if(mpLocalCtx) {
		mpLocalCtx->close();
		delete mpLocalCtx;
		mpLocalCtx = nullptr;
	}
}

//
int ReHttpServer::setUrlRegLam(http_method method, const std::string& pattern, std::function<void()> lis) {
	setUrlReg(method, pattern, [lis](){
		return new GenericUrlCtrl;
	});
	return 0;
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

//	svr.setUrlReg<urlctrl>(HTTP_GET, "/asdfasf", 10, 20);

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
