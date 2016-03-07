/*
 * SharedConnection.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */
#define LOG_LEVEL LOG_VERBOSE
#include "flog.h"

#include "SharedConnection.h"

namespace cahttp {

SharedConnection::SharedConnection() {
	mHandle = 0;
}

SharedConnection::~SharedConnection() {
}


void SharedConnection::OnIdle() {
	ald("connection idle,...");
	mLis(mHandle);
}


void SharedConnection::OnDisconnected() {
	ald("disconnected, ...");
	mLis(mHandle);
	mHandle = 0;
}

void SharedConnection::setRelLis(std::function<void(uint32_t)> lis) {
	mLis = lis;
}

} /* namespace cahttp */
