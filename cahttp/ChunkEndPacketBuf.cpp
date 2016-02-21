/*
 * ChunkEndPacketBuf.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */

#include <string.h>
#include "TEEndPacketBuf.h"

namespace cahttp {

TEEndPacketBuf::TEEndPacketBuf() {
	memcpy(mLineEnd, "0\r\n\r\n", 5);
	mLen = 5;
	setType(0);
}

TEEndPacketBuf::~TEEndPacketBuf() {
}

size_t TEEndPacketBuf::remain() {
	return mLen;
}

void TEEndPacketBuf::consume() {
	mLen = 0;
}

std::pair<size_t, const char*> TEEndPacketBuf::getBuf() {
	if(mLen) {
		return {mLen, mLineEnd};
	} else {
		return {0, nullptr};
	}
}

} /* namespace cahttp */
