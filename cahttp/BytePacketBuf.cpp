/*
 * BytePacketBuf.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "BytePacketBuf.h"
namespace cahttp {

BytePacketBuf::BytePacketBuf() {
	mBuf = nullptr;
	mBufSize = 0;
	mDataSize = 0;
}

BytePacketBuf::~BytePacketBuf() {
	if(mBuf) {
		free(mBuf); mBuf = nullptr;
	}
}

size_t BytePacketBuf::remain() {
	return mDataSize;
}

void BytePacketBuf::consume() {
	mDataSize = 0;
}

std::pair<size_t, const char*> BytePacketBuf::getBuf() {
	if(mDataSize) {
		return {mDataSize, mBuf};
	} else {
		return {0, nullptr};
	}
}

void BytePacketBuf::allocBuf(size_t size) {
	if(mBuf) free(mBuf);
	mBuf = (char*)malloc(size);
	if(mBuf) {
		mBufSize = size;
	} else {
		assert(0);
	}
}

void BytePacketBuf::setData(const char* ptr, size_t len) {
	if(mBufSize<len) {
		allocBuf(len);
	}
	if(mBuf) {
		memcpy(mBuf, ptr, len);
		mDataSize = len;
	}
}

void BytePacketBuf::addData(const char* ptr, size_t len) {
	if(mDataSize+len <= mBufSize) {
		memcpy(mBuf+mDataSize, ptr, len);
	}
}


} /* namespace cahttp */
