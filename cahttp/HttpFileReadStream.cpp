/*
 * HttpFileReadStream.cpp
 *
 *  Created on: Jul 28, 2015
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include "flog.h"
#include "HttpFileReadStream.h"

using namespace std;

namespace cahttp {
HttpFileReadStream::HttpFileReadStream() {
	mReadCnt = 0;
	mDataSize = 0;
	mFileSize = 0;
	mBuf = nullptr;
	mBufSize = 2.5*1024*1024;
	mChunkReadPos = 0;
	mChunkRemain = 0;
}

HttpFileReadStream::~HttpFileReadStream() {
	if(mBuf) {
		free(mBuf);
	}
}

#if 0
string HttpFileReadStream::readStr(size_t len) {
	size_t rcnt;
	if(len<0)
		rcnt = remain();
	else
		rcnt = min(remain(), len);
	mBuf.clear();
	mBuf.resize(rcnt);
	rcnt = mFile.readFile((char*)mBuf.data(), rcnt);
	if(rcnt>0) {
		mReadCnt += rcnt;
		mBuf.resize(rcnt);
	} else {
		mBuf.clear();
	}
	return move(mBuf);
}
#endif

size_t HttpFileReadStream::remain() {
	return (mDataSize-mReadCnt);
}


int HttpFileReadStream::open(const char* path, size_t chksize) {
	mBufSize = chksize;
	mBuf = (char*)malloc(chksize);
	if(mBuf == nullptr) {
		ale("### Error: memory allocation fail...");
		assert(0);
		return -1;
	}
	int fd = mFile.openFile(path, EdFile::OPEN_READ);
	mFileSize = mDataSize = mFile.getSize(path);
	mReadCnt = 0;
	ald("file open=%d, size=%ld", fd, mDataSize);
	return fd;
}

std::pair<const char*, int64_t> HttpFileReadStream::getDataPtr() {
	if(mChunkRemain==0) {
		mChunkReadPos = 0;
		auto rcnt = mFile.readFile(mBuf, mBufSize);
		if(rcnt<0) mChunkRemain=0;
		else mChunkRemain = rcnt;
	}
//	ali("get data ptr");
	ald("chunk readpos=%d, chunkremain=%d", mChunkReadPos, mChunkRemain);
	return {mBuf+mChunkReadPos, mChunkRemain};
}

void HttpFileReadStream::consume(size_t len) {
	mReadCnt += len;
	ald("consume: chunk remain=%d, readcnt=%ld", mChunkRemain, mReadCnt);
	assert(mChunkRemain==len);
	long rm = mChunkRemain - len;
	if(rm >= 0) {
		mChunkRemain = rm;
		mChunkReadPos += len;
	} else {
		mChunkRemain = 0;
		mChunkReadPos = 0;
		mFile.seek(-rm, SEEK_CUR);
	}

}

void HttpFileReadStream::setRange(size_t offset, size_t len) {
	if(mFile.getFd()>=0) {
		mFile.seek(offset, SEEK_SET);
		mDataSize = min( mFileSize - offset, len);
	} else {
		ale("### Error: file not opened...");
		assert(0);

	}
}

ssize_t HttpFileReadStream::store(const char* ptr, size_t len) {
	return -1;
}

void HttpFileReadStream::close() {
	mFile.closeFile();
}
}
