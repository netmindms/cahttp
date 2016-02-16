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


class ReHttpServer {
	friend class svr2_svr_Test;
	friend class ReHttpSvrCtx;
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
	template<typename T, typename ...ARG> int setUrlReg(http_method method, const std::string& pattern, ARG...args) {
		return setUrlReg(method, pattern, [args...]() -> T* {
			return new T(args...);
		});
	}

#ifdef GTEST_BUILD
	static int test();
#endif

private:
	std::list<std::pair<http_method, RegAllocList>> mMethodMap;
	ReUrlCtrl* allocUrlCtrl(http_method method, const std::string& path);

};

} /* namespace cahttp */

#endif /* CAHTTP_REHTTPSERVER_H_ */
