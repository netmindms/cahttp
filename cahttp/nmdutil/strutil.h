/*
 * strutil.h
 *
 *  Created on: Mar 28, 2015
 *      Author: netmind
 */

#ifndef UTIL_STRUTIL_H_
#define UTIL_STRUTIL_H_

#include <string>
#include <vector>
#include <unordered_map>

namespace nmdu {

extern const std::string CONST_NULLSTR;

std::string& Rtrim(std::string &s, const std::string& dels="\r\n ");
std::string Rtrim(std::string &&s, const std::string& dels="\r\n ");
std::string Rtrim(const std::string &s, const std::string& dels="\r\n ");
std::string& Ltrim(std::string &s, const std::string& dels="\r\n ");
std::string Ltrim(const std::string &s, const std::string& dels="\r\n ");
std::string Ltrim(std::string &&s, const std::string& dels="\r\n ");
std::string GetHttpTimeDateNow();

bool IsNumeric(const std::string &s, char ctx='d');
std::vector<std::string> SplitString(const std::string &src, char delc);
std::vector<std::string> SplitString(const std::string &src, const std::string &dels);
std::string DumpVectorStr(std::vector<std::string> &vs, bool vertical=false);
std::string DumpMapStr(const std::unordered_map<std::string, std::string> &ms);
std::pair<std::string, std::string> ParseKeyVal(const char *str, size_t len, char delc, bool remove_whs=false);
std::string ToHex8(uint8_t a);
std::string ToHexs(uint16_t a);
std::string ToHexi(uint32_t a);

template<class T> void ToHexStr(char *buf, T a) {
	uint8_t* ptr = (uint8_t*)&a + sizeof(a) - 1;
	uint8_t n;
	uint8_t* wptr = (uint8_t*)buf;
	for(uint8_t i=0;i<sizeof(a);i++) {
		n = (*ptr)>>4;
		if(n < 10 ) *wptr = n + '0';
		else *wptr = 'a'+n-10;
		wptr++;

		n = (*ptr) & 0x0f;
		if(n < 10 ) *wptr = n + '0';
		else *wptr = 'a'+n-10;
		wptr++;

		ptr--;
	}
	*wptr = 0;
}
}

#endif /* UTIL_STRUTIL_H_ */
