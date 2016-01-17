/*
 * HttpTrEncReadStream.cpp
 *
 *  Created on: Oct 20, 2015
 *      Author: netmind
 */

#include "HttpTrEncReadStream.h"
namespace cahttp {
HttpTrEncReadStream::HttpTrEncReadStream() {
	// TODO Auto-generated constructor stub

}

HttpTrEncReadStream::~HttpTrEncReadStream() {
	// TODO Auto-generated destructor stub
}

std::pair<const char*, int64_t> HttpTrEncReadStream::getDataPtr() {
}

size_t HttpTrEncReadStream::remain() {
}

void HttpTrEncReadStream::consume(size_t len) {
}

void HttpTrEncReadStream::putData(const char* ptr, size_t len) {
	auto remain = mBufSize - mWi;
	if(remain<len) {

	}
}
}
