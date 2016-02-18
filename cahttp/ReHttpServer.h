/*
 * ReHttpServer.h
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REHTTPSERVER_H_
#define CAHTTP_REHTTPSERVER_H_

#include <utility>
#include <list>
#include <functional>
#include <gtest/gtest.h>

#include "http_parser.h"
#include "ReUrlCtrl.h"
#include "RegExp.h"

namespace cahttp {

class ReHttpSvrCtx;

class ReHttpServer {
	friend class svr2_svr_Test;
	friend class ReHttpSvrCtx;
	friend class ReSvrCnn;
public:
	typedef std::function<ReUrlCtrl* ()> UrlAlloctor;
	struct reg_alloc_t {
		RegExp rexp;
		UrlAlloctor alloc;
	};
	typedef std::list<reg_alloc_t> RegAllocList;

	ReHttpServer();
	virtual ~ReHttpServer();

	int setUrlReg(http_method method, const std::string& pattern, UrlAlloctor);
	int setUrlRegLam(http_method method, const std::string& pattern, std::function<void()> lis);
	template<typename T, typename ...ARG> int setUrlReg(http_method method, const std::string& pattern, ARG...args) {
		return setUrlReg(method, pattern, [args...]() -> T* {
			return new T(args...);
		});
	}
//	template<typename T> int setUrlReg(http_method method, const std::string& pattern) {
//		return setUrlReg(method, pattern, []() -> T* {
//			return new T;
//		});
//	}

	int start(int tasknum);
	void close();

private:
	std::list<std::pair<http_method, RegAllocList>> mMethodMap;
	ReUrlCtrl* allocUrlCtrl(http_method method, const std::string& path);
	edft::EdSocket mSocket;
	ReHttpSvrCtx *mpLocalCtx;

#ifdef UNIT_TEST
public:
	static int test();
#endif

};

} /* namespace cahttp */

#endif /* CAHTTP_REHTTPSERVER_H_ */
