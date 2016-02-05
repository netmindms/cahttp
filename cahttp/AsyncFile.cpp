/*
 * AsyncFile.cpp
 *
 *  Created on: Feb 4, 2016
 *      Author: netmind
 */

#include "AsyncFile.h"

using namespace std;
using namespace edft;

namespace cahttp {

AsyncFile::AsyncFile() {
	mChkSize = 2048;
	mSt = nullptr;
	mDataSize = 0;
	mBufLen = 0;
}

AsyncFile::~AsyncFile() {
	close();
}

int AsyncFile::open(const std::string& path, Lis lis) {
	mLis = lis;
	mSt = fopen(path.data(), "rb");
	if(!mSt) {
		return -1;
	}

	mEvt.setOnListener([this](EdEventFd &efd, int cnt) {
		auto *ptr = new char[mChkSize];
		if(ptr) {
			auto rcnt = fread(ptr, 1, mChkSize, mSt);
			if(rcnt>0) {
				mLis(unique_ptr<char[]>(ptr), rcnt);
				mEvt.raise();
			}
		}
	});
	auto fd = mEvt.open();
	if(fd>=0) {
		mEvt.raise();
	}
	return fd>0?0:-1;
}

void AsyncFile::close() {
	mEvt.close();
	if(mSt) {
		fclose(mSt);
		mSt = nullptr;
	}
}

} // namespace cahttp
