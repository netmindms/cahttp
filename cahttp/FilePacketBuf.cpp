/*
 * FilePacketBuf.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include "FilePacketBuf.h"
#include "ext/nmdutil/FileUtil.h"
#include "flog.h"
#define BUF_SIZE 4096

using namespace cahttpu;

namespace cahttp {

FilePacketBuf::FilePacketBuf() {
	mSt = nullptr;
	mBuf = nullptr;
	mBufSize = 0;
	mDataCnt = 0;
	mFileSize = 0;
	mConsumeCnt = 0;
}

FilePacketBuf::~FilePacketBuf() {
	close();
}

size_t FilePacketBuf::remain() {
	return (mFileSize - ftell(mSt) + mDataCnt);
}

void FilePacketBuf::consume() {
	mConsumeCnt += mDataCnt;
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
		ald("consume cnt=%ld, filesize=%ld", mConsumeCnt, mFileSize);
		return {0, nullptr};
	}
}

int FilePacketBuf::open(const std::string& path) {
	mConsumeCnt = 0;
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

	if(mBuf) {
		delete[] mBuf; mBuf = nullptr;
	}
}

} /* namespace cahttp */
