/*
 * ReServTask.h
 *
 *  Created on: Feb 28, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_RESERVTASK_H_
#define CAHTTP_RESERVTASK_H_
#include <ednio/EdNio.h>
#include "ReHttpSvrCtx.h"

namespace cahttp {
class ReHttpServer;

class ReServTask : public edft::EdTask {
public:
	enum {
		UM_NEWCNN=(EDM_USER+1),
		UM_CLOSE_ALLCNN,
	};

	ReServTask(ReHttpServer* serv, int id);
	virtual ~ReServTask();
private:
	ReHttpSvrCtx mCnnCtx;
	ReHttpServer* mpSvr;
	int mId;

	int OnEventProc(edft::EdMsg& msg) override;
};

} /* namespace cahttp */

#endif /* CAHTTP_RESERVTASK_H_ */
