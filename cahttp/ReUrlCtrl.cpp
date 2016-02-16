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

ReUrlCtrl::ReUrlCtrl() {
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
} /* namespace cahttp */
