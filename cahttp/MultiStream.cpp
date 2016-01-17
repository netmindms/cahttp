/*
 * MultiStream.cpp
 *
 *  Created on: Jan 17, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include <assert.h>
#include "MultiStream.h"

namespace cahttp {

MultiStream::MultiStream() {
}

MultiStream::~MultiStream() {
}

void MultiStream::addStrm(HttpBaseReadStream* pstrm) {
	mStrmList.push_back(pstrm);
}

std::pair<const char*, size_t> MultiStream::getDataPtr() {
	if(mStrmList.empty()==false) {
		auto *pstrm = mStrmList.front();
		return pstrm->getDataPtr();
	} else {
		return {nullptr, -1};
	}
}

void MultiStream::consume(size_t len) {
	if(mStrmList.empty()==false) {
		auto *pstrm = mStrmList.front();
		pstrm->consume(len);
		if(pstrm->remain()==0) {
			mStrmList.pop_front();
		}
	} else {
		assert(0);
	}
}

} /* namespace cahttp */
