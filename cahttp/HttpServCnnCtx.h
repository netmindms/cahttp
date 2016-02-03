/*
 * HttpServCnnMan.h
 *
 *  Created on: Nov 2, 2015
 *      Author: root
 */

#ifndef EXTERNAL_CAHTTP_HTTPSERVCNNCTX_H_
#define EXTERNAL_CAHTTP_HTTPSERVCNNCTX_H_

#include <unordered_map>
#include "ServCnn.h"
#include "CaHttpUrlCtrl.h"
#include "SimpleHandle.h"
namespace cahttp {
class CaHttpServer;

class HttpServCnnCtx {
	friend class ServTask;
	friend class ServCnn;
	friend class CaHttpServer;
private:
	HttpServCnnCtx();
	virtual ~HttpServCnnCtx();
private:
	void init(CaHttpServer &svr, int maxn);
	void newCnn(int fd);
	void close();
	void freeCnn(uint32_t handle);
	void disconnectedCnn(ServCnn* pcnn);
	const UrlMap* getUrlMap(http_method method);
	CaHttpUrlCtrl* findAndAlloc(http_method method, const std::string& urlstr);

	uint32_t mHandleSeed;
	std::unordered_map<uint32_t, ServCnn> mCnnList;
	CaHttpServer *mpSvr;
	SimpleHandle mHandleFac;
};
}
#endif /* EXTERNAL_CAHTTP_HTTPSERVCNNCTX_H_ */
