/*
 * CBuffer.cpp
 *
 *  Created on: Sep 10, 2015
 *      Author: netmind
 */



#include <string.h>
#include <cassert>
#include <cstdlib>
#include <memory>

#include "CBuffer.h"
#include "nmdtype.h"
namespace nmdu {

CBuffer::CBuffer() {
	mBuf = nullptr;
	mCapacity = 0;
	mSize = 0;
}

CBuffer::~CBuffer() {
	if(mBuf) {
		free(mBuf);
		mBuf = nullptr;
	}

}

size_t CBuffer::size() {
	return mSize;
}

size_t CBuffer::capacity() {
	return mCapacity;
}

CBuffer::CBuffer(CBuffer&& other) {
	*this = move(other);
}

CBuffer& CBuffer::operator =(CBuffer&& other) {
	if(this != &other) {
		mBuf = other.mBuf;
		mCapacity = other.mCapacity;
		mSize = other.mSize;

		other.mBuf = nullptr;
		other.mCapacity = 0;
		other.mSize = 0;
	}
	return *this;
}

void CBuffer::append(const char* ptr, size_t len) {
	if(mSize+len>mCapacity) {
		auto tptr = (char*)realloc(mBuf, mSize+len);
		if(tptr) {
			memcpy(tptr+mSize, ptr, len);
			mCapacity = mSize+len;
			mSize += len;
			mBuf = tptr;

		}
		else {
			assert(0);
		}
	}
	else {
		memcpy(mBuf+mSize, ptr, len);
		mSize += len;
	}
}


const char* CBuffer::get() {
	return mBuf;
}


void CBuffer::assign(upChar ptr, size_t len) {
	if(mBuf) free(mBuf);
	mBuf = ptr.release();
	mCapacity = len;
	mSize = len;
}


char* CBuffer::release() {
	auto ptr = mBuf;
	mBuf = nullptr;
	return ptr;
}

} /* namespace nmdu */
