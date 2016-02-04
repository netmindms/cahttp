/*
 * CaHttpServer.h
 *
 *  Created on: Apr 24, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPSERVER_H_
#define SRC_CAHTTPSERVER_H_

#include <functional>
#include <regex>
#include <vector>
#include <utility>
#include <unordered_map>
#include <ednio/EdNio.h>
#include "CaHttpCommon.h"
#include "CaHttpReq.h"
#include "CaHttpUrlCtrl.h"
#include "ServTask.h"

namespace cahttp {

class HttpServCnnCtx;
class CaHttpServer {
	friend class ServTask;
	friend class ServCnn;
	friend class HttpServCnnCtx;
public:
	typedef std::function<void (CaHttpSvrReq& req, int)> UrlLis;

	enum {
		SVRE_NEW_CONNECTION,
		SVRE_CLOSE_CONNECTION,
	};
	typedef std::function<void (int, ServCnn&)> Lis;

	CaHttpServer();
	virtual ~CaHttpServer();
	void setTaskNum(int num);


	template<class T> void setUrl(http_method method, const std::string &url) {
		setUrl<T>(method, std::string(url));
	};
	template<class T> void setUrl(http_method method, std::string &&url) {
		UrlMap* pmap=nullptr;
		for(auto &mi: mMethodUrlVec) {
			if(mi.first == method) {
				pmap = &mi.second;
			}
		}
		if(!pmap) {
			mMethodUrlVec.emplace_back();
			auto &mm = mMethodUrlVec.back();
			mm.first = method;
			pmap = &(mm.second);
		}
		pmap->emplace(move(url),  []() -> T*{
			return new T;
		});
	};

	void setUrl(http_method method, const std::string &url, UrlLis lis);

#ifdef CAHTTP_REGEX_URLPATTERN
	template<class T> void setUrlRegEx(http_method method, std::regex &&ex) {
		UrlRegExMap *pm=findMethodRegExMap(method);
		if(!pm) {
			// add new regex url map
			mMethodRegVec.emplace_back();
			pm = &(mMethodRegVec.back().second);
			mMethodRegVec.back().first = method;
		}
		std::pair<std::regex, UrlCtrlAlloc> reg = { std::move(ex),
			[]() -> T* {
				return new T;
			}};
		pm->emplace_back(std::move(reg));

//		mUrlRegExMap.emplace_back(move(reg));

	};
	template<class T> void setUrlRegEx(http_method method, const std::regex &ex) {
		std::regex tex = ex;
		setUrlRegEx<T>(method, move(tex));
	};

	template<class T> void setUrlRegEx(http_method method, const std::string &rstr) {
		setUrlRegEx<T>(method, std::regex(rstr));
	};
#endif

	int start(int task_num);
	void close();
	const UrlMap* getUrlMap(http_method method);
	UrlCtrlAlloc matchRegExUrl(http_method method, std::smatch &result, const std::string& s);
	void config(const char* param, const char *val);
	void setOnListener(Lis lis);
	void closeAllConnections();
	void dump();
private:
	int mTaskNum;
	std::vector<ServTask*> mTasks;
	std::string mIp;
	int mPort;
	int mCfgCnnTimeout;
	EdSocket mLisSock;
	uint32_t mJobOrder;
	std::vector<std::pair<http_method, UrlMap>> mMethodUrlVec;
	UrlRegExMap mUrlRegExMap;
	std::vector<std::pair<http_method, UrlRegExMap>> mMethodRegVec;
	Lis mLis;
	HttpServCnnCtx* mpCnnCtx;

	void init_tasks(int task_num);
	void notifyNewConnection(ServCnn& cnn);
	void notifyCloseConnection(ServCnn& cnn);
	UrlRegExMap* findMethodRegExMap(http_method method);
};
}
#endif /* SRC_CAHTTPSERVER_H_ */
