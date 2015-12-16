/*
 * HttpFileWriterStream.cpp
 *
 *  Created on: Jul 28, 2015
 *      Author: netmind
 */
#include "HttpFileWriteStream.h"

#include "flog.h"
namespace cahttp {
HttpFileWriteStream::HttpFileWriteStream() {
	mSize = 0;
}

HttpFileWriteStream::~HttpFileWriteStream() {
}

ssize_t HttpFileWriteStream::write(const char* buf, size_t len) {
	// TODO : ignore file writing error
	auto wcnt = mFile.writeFile(buf, len);
	if(wcnt!= len) {
		ale("### Error: file writing error");
	}
	mSize += len;
	return len;
}

size_t HttpFileWriteStream::size() {
	return mSize;
}

void HttpFileWriteStream::end() {
	mFile.closeFile();
}

int HttpFileWriteStream::open(const char* path) {
	mSize = 0;
	return mFile.openFile(path, EdFile::OPEN_RWTC);
}
}
