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
	enum {
		SVRE_NEW_CONNECTION,
		SVRE_CLOSE_CONNECTION,
	};
	typedef function<void (int, ServCnn&)> Lis;

	CaHttpServer();
	virtual ~CaHttpServer();
	void setTaskNum(int num);

	template<class T> void setUrl(http_method method, const string &url) {
		setUrl<T>(method, string(url));
	};
	template<class T> void setUrl(http_method method, string &&url) {
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

#ifdef CAHTTP_REGEX_URLPATTERN
	template<class T> void setUrlRegEx(std::regex &&ex) {
		pair<regex, UrlCtrlAlloc> reg = { move(ex),
			[]() -> T* {
				return new T;
			}};
		mUrlRegExMap.emplace_back(move(reg));
	};
	template<class T> void setUrlRegEx(const std::regex &ex) {
		std::regex tex = ex;
		setUrlRegEx<T>(move(tex));
	};
	template<class T> void setUrlRegEx(const string &rstr) {
		setUrlRegEx<T>(regex(rstr));
	};
#endif

	int start(int task_num);
	void close();
	const UrlMap* getUrlMap(http_method method);
	UrlCtrlAlloc matchRegExUrl(smatch &result, const string& s);
	void config(const char* param, const char *val);
	void setOnListener(Lis lis);
	void closeAllConnections();
	void dump();
private:
	int mTaskNum;
	vector<ServTask*> mTasks;
	string mIp;
	int mPort;
	int mCfgCnnTimeout;
	EdSocket mLisSock;
	uint32_t mJobOrder;
	std::vector<std::pair<http_method, UrlMap>> mMethodUrlVec;
	UrlRegExMap mUrlRegExMap;
	Lis mLis;
	HttpServCnnCtx* mpCnnCtx;

	void init_tasks(int task_num);
	void notifyNewConnection(ServCnn& cnn);
	void notifyCloseConnection(ServCnn& cnn);
};
}
#endif /* SRC_CAHTTPSERVER_H_ */
