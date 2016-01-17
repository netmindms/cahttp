/*
 * ServTask.h
 *
 *  Created on: Jul 19, 2015
 *      Author: netmind
 */

#ifndef SRC_SERVTASK_H_
#define SRC_SERVTASK_H_

#include <ednio/EdNio.h>
#include <list>

#include "CaHttpUrlCtrl.h"
#include "ServCnn.h"
#include "HttpServCnnCtx.h"

using namespace edft;

namespace cahttp {
class CaHttpServer;

class ServTask : public EdTask {
	friend class ServCnn;
public:
	enum {
		UM_NEWCNN=(EDM_USER+1),
		UM_CLOSE_ALLCNN,
	};
	ServTask(CaHttpServer*);
	virtual ~ServTask();
	int OnEventProc(EdMsg& msg) override final;
	const UrlMap& getUrlMap();
	CaHttpServer& getServer();
private:
	CaHttpServer *mServer;
	std::unordered_map<uint32_t, ServCnn> mCnnList;
	HttpServCnnCtx mCnnCtx;

	ServCnn& newCnn(int port);
	void closeAllCnn();
	void freeCnn(uint32_t handle);
};
}
#endif /* SRC_SERVTASK_H_ */
