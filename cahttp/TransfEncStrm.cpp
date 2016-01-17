/*
 * TransfEncStrm.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: netmind
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "TransfEncStrm.h"

#define CHUNK_SIZE 4096

namespace cahttp {

TransfEncStrm::TransfEncStrm() {
	mBuf = nullptr;
	mChunkSize = 0;
	mpOrgStrm = nullptr;
	mBufSize = 0;
}

TransfEncStrm::~TransfEncStrm() {
	if(mBuf) {
		free(mBuf);
	}
}

std::pair<const char*, int64_t> TransfEncStrm::getDataPtr() {
	if(mChunkSize) {
		return {mBuf, mChunkSize};
	} else {
		if(mpOrgStrm) {
			auto dp = mpOrgStrm->getDataPtr();
			if(dp.second>=0) {
				auto bs = sprintf(mBuf, "%lx\r\n", dp.second);
				auto pktsize = bs + dp.second + 2;
				if((size_t)pktsize> mBufSize) {
					auto allocsize = pktsize*1.5;
					mBuf = (char*)realloc(mBuf, allocsize);
					if(!mBuf) {
						assert(0);
						return {nullptr, -1};
					}
					mBufSize = allocsize;
				}

				memcpy(mBuf+bs, dp.first, dp.second);
				bs += dp.second;
				mBuf[bs++] = '\r', mBuf[bs++]='\n';
				mChunkSize = bs;
				mpOrgStrm->consume(dp.second);
				if(dp.second == 0) {
					mpOrgStrm = nullptr;
				}
				return {mBuf, mChunkSize};
			} else {
				return dp;
			}
		} else {
			return {nullptr, 0};
		}
	}
}

size_t TransfEncStrm::remain() {
	return -1;
}

void TransfEncStrm::consume(size_t len) {
	assert(mChunkSize == len);
	mChunkSize = 0;
}

ssize_t TransfEncStrm::store(const char* ptr, size_t len) {
	return mpOrgStrm->store(ptr, len);
}

void TransfEncStrm::setStream(HttpBaseReadStream* pstrm) {
	if(!mBuf) {
		mBufSize = 4*1024+128;
		mBuf = (char*)malloc(mChunkSize);
		assert(mBuf);
	}
	mpOrgStrm = pstrm;
}

} /* namespace cahttp */
