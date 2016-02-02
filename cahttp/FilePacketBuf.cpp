/*
 * FilePacketBuf.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: netmind
 */

#include "FilePacketBuf.h"
#include "ext/nmdutil/FileUtil.h"
#define BUF_SIZE 4096

using namespace nmdu;

namespace cahttp {

FilePacketBuf::FilePacketBuf() {
	mSt = nullptr;
	mBuf = nullptr;
	mBufSize = 0;
	mDataCnt = 0;
	mFileSize = 0;
}

FilePacketBuf::~FilePacketBuf() {
	close();
}

size_t FilePacketBuf::remain() {
	return (ftell(mSt) - mFileSize + mDataCnt);
}

void FilePacketBuf::consume() {
	mDataCnt = 0;
}

std::pair<size_t, const char*> FilePacketBuf::getBuf() {
	if(mDataCnt==0) {
		auto rcnt = fread(mBuf, 1, mBufSize, mSt);
		if(rcnt>0) {
			mDataCnt = rcnt;
		}
	}
	if(mDataCnt>0) {
		return {mDataCnt, mBuf};
	} else {
		return {0, nullptr};
	}
}

int FilePacketBuf::open(const std::string& path) {
	mFileSize = FileUtil::getSize(path.data());
	mSt = fopen(path.data(), "rb");
	if(mSt) {
		if(mBuf==nullptr) {
			mBuf = new char[BUF_SIZE];
			mBufSize = BUF_SIZE;
		}
		return 0;
	} else {
		return -1;
	}
}

void FilePacketBuf::close() {
	if(mSt) {
		fclose(mSt); mSt = nullptr;
	}
}

} /* namespace cahttp */
