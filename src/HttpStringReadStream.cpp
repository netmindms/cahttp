/*
 * HttpStringReadStream.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: netmind
 */

#include "HttpStringReadStream.h"

using namespace std;
namespace cahttp {
HttpStringReadStream::HttpStringReadStream() {
	// TODO Auto-generated constructor stub
	mReadPos = 0;
}

HttpStringReadStream::~HttpStringReadStream() {
	// TODO Auto-generated destructor stub
}

#if 0
string HttpStringReadStream::readStr(size_t len) {
	if(mReadPos==0 && ( len<0 || len>=mStrBuf.size())) {
		mReadPos = mStrBuf.size();
		string rs = move(mStrBuf);
		mStrBuf.clear();
		mReadPos = 0;
		return move(rs);
	} else {
		auto rcnt = min(len, mStrBuf.size()-mReadPos);
		string rs(mStrBuf.data()+mReadPos, rcnt);
		mReadPos += rcnt;
		return move(rs);
	}
}
#endif

size_t HttpStringReadStream::remain() {
	return (mStrBuf.size()-mReadPos);
}

void HttpStringReadStream::setString(string&& s) {
	mReadPos = 0;
	mStrBuf = move(s);
}

std::pair<const char*, int64_t> HttpStringReadStream::getDataPtr() {
	return {mStrBuf.data()+mReadPos, mStrBuf.size()-mReadPos};
}

void HttpStringReadStream::consume(size_t len) {
	auto cnt = min(mStrBuf.size()-mReadPos, len);
	mReadPos += cnt;
}

ssize_t HttpStringReadStream::store(const char* ptr, size_t len) {
	mStrBuf.append(ptr, len);
	return len;
}

const string& HttpStringReadStream::getString() {
	return mStrBuf;
}
}
