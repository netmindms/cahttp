/*
 * ServTask.cpp
 *
 *  Created on: Jul 19, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_INFO

#include "flog.h"
#include "ServTask.h"

#include "CaHttpServer.h"

namespace cahttp {
ServTask::ServTask(CaHttpServer* svr) {
	mServer = svr;
}

ServTask::~ServTask() {
}

int ServTask::OnEventProc(EdMsg& msg) {
	if (msg.msgid == EDM_INIT) {
		ali("http service task starts");
		mCnnCtx.init(*mServer, 0);
	}
	else if (msg.msgid == UM_NEWCNN) {
		ali("new cnn fd=%d", msg.p1);
		mCnnCtx.newCnn(msg.p1);
	}
	else if(msg.msgid == UM_CLOSE_ALLCNN) {
		ali("  close all server connections,...");
		mCnnCtx.close();
	}
	else if (msg.msgid == EDM_CLOSE) {
		ali("http service task closing...");
		mCnnCtx.close();
	}
	return 0;
}
}
