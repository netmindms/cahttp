/*
 * StringPacketBuf.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#include "StringPacketBuf.h"

namespace cahttp {

StringPacketBuf::StringPacketBuf() {

}

StringPacketBuf::StringPacketBuf(const std::string& s) {
	mString = s;
}

StringPacketBuf::~StringPacketBuf() {
}

void StringPacketBuf::setString(std::string&& s) {
	mString = move(s);
}

void StringPacketBuf::setString(const char* ptr, size_t len) {
	mString.assign(ptr, len);
}


void StringPacketBuf::consume() {
	mString.clear();
}


size_t StringPacketBuf::remain() {
	return mString.size();
}


std::pair<size_t, const char*> StringPacketBuf::getBuf() {
	return {mString.size(), mString.data()};
}

} /* namespace cahttp */
