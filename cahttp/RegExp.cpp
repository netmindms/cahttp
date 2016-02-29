/*
 * RegExp.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: netmind
 */

#include "RegExp.h"

using namespace std;

namespace cahttp {

RegExp::RegExp() {
	mpRegex = nullptr;
}

RegExp::~RegExp() {
	if(mpRegex) {
		regfree(mpRegex);
	}
}

int RegExp::setPattern(const char* pattern) {
	if(mpRegex) {
		regfree(mpRegex); mpRegex=nullptr;
	}
	auto r = regcomp(&mRegex, (string("^")+pattern+"$").c_str(), REG_EXTENDED);
	if(!r) {
		mpRegex = &mRegex;
	}
	return r;
}

pair<int, vector<string>> RegExp::matchParams(const char* str) {
	int r;
	vector<string> vs;
	size_t off;
	regmatch_t ma[5];
	const char* ptr = str;
	for(;;) {
		r = regexec(mpRegex, ptr, sizeof(ma)/sizeof(regmatch_t), ma, 0);
		if(r) break;
		for(int i=0;i<10;i++) {
			if(ma[i].rm_eo==-1) {
				break;
			}
			if(i!=0) {
				vs.emplace_back(ptr+ma[i].rm_so, ma[i].rm_eo- ma[i].rm_so);
			} else {
				off = ma[i].rm_eo;
			}
		}
		ptr += off;
	}
	return {(ptr!=str)?0:-1, move(vs)};
}

std::pair<int, std::vector<std::string> > RegExp::matchParams(const std::string& str) {
	return matchParams(str.c_str());
}

void RegExp::clear() {
	if(mpRegex) {
		regfree(mpRegex); mpRegex=nullptr;
	}
}

} /* namespace cahttp */
