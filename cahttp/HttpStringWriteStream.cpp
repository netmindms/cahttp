/*
 * HttpStringWriteStream.cpp
 *
 *  Created on: Jul 24, 2015
 *      Author: netmind
 */

#include "HttpStringWriteStream.h"

using namespace std;
namespace cahttp {
HttpStringWriteStream::HttpStringWriteStream() {

}

HttpStringWriteStream::~HttpStringWriteStream() {
}

ssize_t HttpStringWriteStream::write(const char* buf, size_t len) {
	mStr.append(buf, len);
	return len;
}

size_t HttpStringWriteStream::size() {
	return mStr.size();
}

string HttpStringWriteStream::fetchString() {
	return move(mStr);
}

const string& HttpStringWriteStream::getString() {
	return mStr;
}
}
