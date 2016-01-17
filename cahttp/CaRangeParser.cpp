/*
 * CaRangeHeader.cpp
 *
 *  Created on: Oct 23, 2015
 *      Author: netmind
 */

#include "CaRangeParser.h"

#include <string>
#include <string.h>
using namespace std;
namespace cahttp {
enum parse_stt_e {
	s_start, s_eq, s_offs, s_offe,
};

CaRangeParser::CaRangeParser() {
	// TODO Auto-generated constructor stub
	mFlag = 0;
	mStart = mEnd = -1;
}

CaRangeParser::~CaRangeParser() {
	// TODO Auto-generated destructor stub
}

int CaRangeParser::parse(const char* str, size_t len) {
	mFlag = 0;
	size_t remain = len;
	const char* ptr = str;
	char ch;
	parse_stt_e stt = s_start;
	string ts;
	int idx;
	int digit_idx;
	const char* cmp_str;
	cmp_str = "bytes=";
	int cmp_len = strlen(cmp_str);
	idx = 0;
	for (; remain > 0;) {
		ch = *ptr++, remain--;
		if (stt == s_start) {
			if (ch == cmp_str[idx]) {
				idx++;
				if (idx == cmp_len) {
					stt = s_offs;
					mStart = 0;
					digit_idx = 0;
				}
			} else {
				return -1;
			}
		} else if (stt == s_offs) {
			if (ch >= '0' && ch <= '9') {
				mStart = mStart*10 + ch - '0';
				digit_idx++;
			} else if (ch == '-') {
				if(digit_idx>0) mFlag |= 0x01;
				stt = s_offe;
				digit_idx = 0;
				mEnd = 0;
			} else {
				return -1;
			}
		} else if (stt == s_offe) {
			if (ch >= '0' && ch <= '9') {
				mEnd = mEnd * 10 + ch - '0';
				digit_idx++;
			} else {
				if(digit_idx>0) mFlag |= 0x02;
			}
		}
	}
	return 0;
}

std::pair<long, long> CaRangeParser::getOffset(size_t total_size) {
	long s, e;
	if(mFlag & 0x01) {
		s = mStart;
		if(mFlag & 0x02) {
			e = mEnd;
		} else {
			e = total_size - 1;
		}
	} else {
		s = total_size - mEnd;
		e = total_size - 1;
	}
	return {s, e};
}
}
