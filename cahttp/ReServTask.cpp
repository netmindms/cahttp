/*
 * ReServTask.cpp
 *
 *  Created on: Feb 28, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_VERBOSE

#include "ReServTask.h"
#include "flog.h"
using namespace std;
using namespace edft;

namespace cahttp {

ReServTask::ReServTask(ReHttpServer* serv, int id): mpSvr(serv) {
	mId = id;
}

ReServTask::~ReServTask() {
}


int ReServTask::OnEventProc(EdMsg& msg) {
	if (msg.msgid == EDM_INIT) {
		ali("http service task starts, id=%d", mId);
//		mCnnCtx.init(*mServer, 0);
		mCnnCtx.init(*mpSvr);
	}
	else if (msg.msgid == UM_NEWCNN) {
		ali("new cnn fd=%d, task_id=%d", msg.p1, mId);
		mCnnCtx.newCnn(msg.p1);
	}
	else if(msg.msgid == UM_CLOSE_ALLCNN) {
		ali("  close all server connections, task_id=%d", mId);
		mCnnCtx.close();
	}
	else if (msg.msgid == EDM_CLOSE) {
		ali("http service task closing...task_id=%d", mId);
		mCnnCtx.close();
	}
	return 0;
}
} /* namespace cahttp */
