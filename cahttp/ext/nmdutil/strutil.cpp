/*
 * strutil.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: netmind
 */

#include <iostream>
#include <string.h>
#include <string>
#include "strutil.h"
using namespace std;

namespace cahttpu {

const string CONST_NULLSTR;

string& Rtrim(string &s, const string& dels)
{
	auto p = s.find_last_not_of(dels);
	if (p != string::npos)
	{
		s.erase(p + 1);
	}
	else
	{
		s.clear();
	}
	return s;
}

string Rtrim(string &&s, const string& dels)
{
	Rtrim(s, dels);
	return move(s);
}

string Rtrim(const string &s, const string& dels)
{
	string rets = s;
	return Rtrim(move(rets));
}

string& Ltrim(string &s, const string& dels)
{
	auto p = s.find_first_not_of(dels);
	if (p != string::npos)
	{
		s.erase(0, p);
	}
	else
	{
		s.clear();
	}
	return s;
}

string Ltrim(const string &s, const string& dels)
{
	string rets = s;
	return Ltrim(move(rets), dels);
}
string Ltrim(string &&s, const string& dels)
{
	Ltrim(s, dels);
	return move(s);
}



vector<string> SplitString(const string &src, const string &dels) {
	vector<string> result;
	size_t pos, cpos;
	size_t len;
	for(pos=0;;) {
		cpos = src.find(dels, pos);
		if(cpos != src.npos) {
			len = cpos - pos;
			if(len>0) {
//				cout << string(src.data()+pos, len) << endl;
				result.emplace_back(src.data()+pos, len);
			}
			pos = cpos + dels.size();
		}
		else {
			if(pos<src.size()) {
				len = src.size() - pos;
				result.emplace_back(src.data()+pos, len);
			}
			break;
		}
	}

	return move(result);
}

#if 1
vector<string> SplitString(const string &src, char delc) {
	vector<string> arr;
	const char* ptr;
	const char* endp;
	size_t rcnt;
	auto remain = src.size();
	bool bs;
	for(bs=false, ptr=src.data();remain>0;) {
		if(!bs) {
			if(*ptr==delc) {
				remain--;
				ptr++;
				continue;
			}
		}

		endp = (const char*)memchr(ptr, delc, remain);
		if(endp) {
			rcnt = endp - ptr;
			arr.emplace_back(ptr, rcnt);
//			rcnt++; // consume delc
		} else {
			rcnt = remain;
			arr.emplace_back(ptr, rcnt);
		}
		remain -= rcnt;
		ptr += rcnt;
		bs = false;
	}
	return move(arr);
}

#else
vector<string> SplitString(const string &src, char delc)
{
	vector<string> arr;
	const char* ptr = src.data();
	auto remain = src.size();
	size_t rcnt;
	for (; remain > 0;)
	{
		const char* endp = (const char*) memchr(ptr, delc, remain);
		if (endp != nullptr)
		{
			rcnt = endp - ptr;
			arr.emplace_back(ptr, rcnt);
			rcnt++; // delc
		}
		else
		{
			rcnt = remain;
			arr.emplace_back(ptr, rcnt);
		}
		remain -= rcnt;
		ptr += rcnt;
	}
	return move(arr);
}
#endif


bool IsNumeric(const string &s, char ctx)
{
	if (s.empty())
		return false;
	if (ctx == 'd')
	{
		for (auto c : s)
		{
			if (!isdigit(c))
				return false;
		}
	}
	else if (ctx == 'h')
	{
		for (auto c : s)
		{
			if (!isxdigit(c))
				return false;
		}
	}
	return true;
}

string DumpVectorStr(vector<string> &vs, bool vertical)
{
	if(vs.size()==0)
		return "";

	string ds;
	string del;
	if(vertical)
		del = "\n";
	else
		del = " ";
	for(auto &s: vs)
	{
		ds += s+del;
	}
	if(vertical==false) ds.pop_back();
	return move(ds);
}

string DumpMapStr(const unordered_map<string, string> &ms) {
	string s;
	for(auto &m: ms) {
		s += m.first + ": " + m.second+"\n";
	}
	return s;
}

pair<string, string> ParseKeyVal(const char *str, size_t len, char delc, bool remove_whs) {
	char* ptr = (char*)memchr((void*)str, (int)delc, len);
	pair<string, string> kv;
	if(!ptr) {
		kv.first.assign(str, len);
	} else {
		kv.first.assign(str, ptr-str);
		kv.second.assign(ptr+1, len - (ptr+1-str));
	}

	if(remove_whs) {
		Rtrim(kv.first);Ltrim(kv.first);
		Rtrim(kv.second);Ltrim(kv.second);
	}

	return move(kv);
}



static char _monthString[][4] ={ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
				"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char _dayString[][4] = {  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
string GetHttpTimeDateNow()
{
	time_t t;
	struct tm tmdata;
	time(&t);
	gmtime_r(&t, &tmdata);
	//Tue, 08 Jul 2014 00:09:47 GMT
	char buf[256];
	auto cnt = sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT", _dayString[tmdata.tm_wday],
			tmdata.tm_mday, _monthString[tmdata.tm_mon], 1900+tmdata.tm_year, tmdata.tm_hour, tmdata.tm_min, tmdata.tm_sec);
	return move(string(buf, cnt));
}

string ToHex8(uint8_t a) {
	string res;
	res.reserve(sizeof(a)*2);
	uint8_t* ptr = (uint8_t*)&a + sizeof(a) - 1;
	uint8_t n;
	for(uint8_t i=0;i<sizeof(a);i++) {
		n = (*ptr)>>4;
		if(n < 10 ) res.push_back(n + '0');
		else res.push_back('a'+10-n);

		n = (*ptr) & 0x0f;
		if(n < 10 ) res.push_back(n + '0');
		else res.push_back('a'+10-n);

		ptr--;
	}
	return res;
}

string ToHexs(uint16_t a) {
	string res;
	res.reserve(sizeof(a)*2);
	uint8_t* ptr = (uint8_t*)&a + sizeof(a) - 1;
	uint8_t n;
	for(uint8_t i=0;i<sizeof(a);i++) {
		n = (*ptr)>>4;
		if(n < 10 ) res.push_back(n + '0');
		else res.push_back('a'+10-n);

		n = (*ptr) & 0x0f;
		if(n < 10 ) res.push_back(n + '0');
		else res.push_back('a'+10-n);

		ptr--;
	}
	return res;
}

string ToHexi(uint32_t a) {
	string res;
	res.reserve(sizeof(a)*2);
	uint8_t* ptr = (uint8_t*)&a + sizeof(a) - 1;
	uint8_t n;
	for(uint8_t i=0;i<sizeof(a);i++) {
		n = (*ptr)>>4;
		if(n < 10 ) res.push_back(n + '0');
		else res.push_back('a'+10-n);

		n = (*ptr) & 0x0f;
		if(n < 10 ) res.push_back(n + '0');
		else res.push_back('a'+10-n);

		ptr--;
	}
	return res;
}



} // namespace

