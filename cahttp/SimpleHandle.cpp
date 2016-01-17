/*
 * SimpleHandle.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: netmind
 */

#include "SimpleHandle.h"

SimpleHandle::SimpleHandle() {
	mSeed = 0;
}

SimpleHandle::~SimpleHandle() {
}

uint32_t SimpleHandle::newHandle() {
	mLock.lock();
	auto h = ++mSeed ? mSeed:++mSeed;
	mLock.unlock();
	return h;
}
