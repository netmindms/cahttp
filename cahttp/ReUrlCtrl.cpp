/*
 * ReUrlCtrl.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include "ReUrlCtrl.h"
#include "flog.h"

namespace cahttp {

ReUrlCtrl::ReUrlCtrl(): mCnnIf(this) {
	mpReqMsg = nullptr;
}

ReUrlCtrl::~ReUrlCtrl() {
	if(mpReqMsg) {
		delete mpReqMsg;
	}
}


std::vector<std::string>& cahttp::ReUrlCtrl::getPathParams() {
	return mPathParams;
}


void ReUrlCtrl::OnMsgHdr(BaseMsg& msg) {
	ald("on msg header");
}

void ReUrlCtrl::OnMsg(BaseMsg& msg) {
}

void ReUrlCtrl::OnEnd() {
	ald("on end...");
}

void ReUrlCtrl::init(upBaseMsg upmsg) {
	mpReqMsg = upmsg.release();
}

void ReUrlCtrl::OnData(std::string&& data) {
}


int ReUrlCtrl::cnnif::OnWritable() {
}

int ReUrlCtrl::cnnif::OnMsg(std::unique_ptr<BaseMsg> upmsg) {
}

int ReUrlCtrl::cnnif::OnData(std::string&& data) {
}

int ReUrlCtrl::cnnif::OnCnn(int cnnstatus) {
}

} /* namespace cahttp */
