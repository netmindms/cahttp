/*
 * SharedConnection.cpp
 *
 *  Created on: Feb 15, 2016
 *      Author: netmind
 */

#include "SharedConnection.h"

namespace cahttp {

SharedConnection::SharedConnection() {
	mHandleSeed = 0;
	mhCurSend = 0;
}

SharedConnection::~SharedConnection() {
	// TODO Auto-generated destructor stub
}

uint32_t SharedConnection::startSend(CnnIf* pif) {
	if(++mHandleSeed==0) ++mHandleSeed;
	mSharedIfs.emplace_back(mHandleSeed, pif);
	if(mhCurSend==0) {
		mhCurSend = mHandleSeed;
	}
	return mHandleSeed;
}

void SharedConnection::endSend(uint32_t handle) {
}

int SharedConnection::send(uint32_t handle, const char* buf, size_t len) {
	if(handle == mhCurSend) {
		return BaseConnection::send(handle, buf, len);
	} else {
		return SEND_NEXT;
	}
}

void SharedConnection::close() {
}



} /* namespace cahttp */
