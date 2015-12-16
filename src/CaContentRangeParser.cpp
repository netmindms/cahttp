/*
 * CaContentRangeParser.cpp
 *
 *  Created on: Oct 23, 2015
 *      Author: netmind
 */


#include <string.h>
#include "CaContentRangeParser.h"

namespace cahttp {


enum parse_state_e {
	s_start, s_offs, s_offe, s_totalsize, s_sp,
};

CaContentRangeParser::CaContentRangeParser() {
	mStart=0;
	mEnd=0;

	mTotalSize = 0;
}

CaContentRangeParser::~CaContentRangeParser() {
	// TODO Auto-generated destructor stub
}

int CaContentRangeParser::parse(const char* str, size_t len) {
	char ch;
	auto remain = len;
	auto stt = s_start;
	auto next_stt = s_start;
	const char *cmp_str = "bytes";
	size_t cmp_len = strlen(cmp_str);
	size_t idx = 0;
	for (; remain > 0;) {
		ch = *str++, remain--;
		if (stt == s_start) {
			if (ch == cmp_str[idx]) {
				idx++;
				if (idx == cmp_len) {
					stt = s_sp;
					next_stt = s_offs;
					mStart = 0;
				}
			} else {
				return -1;
			}
		} else if (stt == s_sp) {
			if (ch != ' ') {
				stt = next_stt;
			}
		} else if (stt == s_offs) {
			if (ch >= '0' && ch <= '9') {
				mStart = mStart * 10 + ch - '0';
			} else if (ch == '-') {
				stt = s_offe;
				mEnd = 0;
			}
		} else if (stt == s_offe) {
			if (ch >= '0' && ch <= '9') {
				mEnd = mEnd * 10 + ch - '0';
			} else if (ch == '/') {
				stt = s_totalsize;
				mTotalSize = 0;
			}
		} else if (stt == s_totalsize) {
			if (ch >= '0' && ch <= '9') {
				mTotalSize = mTotalSize * 10 + ch - '0';
			}
		}
	}
	return 0;
}

void CaContentRangeParser::getOffset(size_t& start, size_t& end, size_t& total) {
	start = mStart;
	end = mEnd;
	total = mTotalSize;
}

}
