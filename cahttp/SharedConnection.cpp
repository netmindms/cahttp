/*
 * SharedConnection.cpp
 *
 *  Created on: Feb 21, 2016
 *      Author: netmind
 */

#include "SharedConnection.h"

namespace cahttp {

SharedConnection::SharedConnection() {
	mHandle = 0;
	mSvrIp = 0;
	mSvrPort = 0;
}

SharedConnection::~SharedConnection() {
	// TODO Auto-generated destructor stub
}

} /* namespace cahttp */
