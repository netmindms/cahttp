/*
 * EdUrlParser.cpp
 *
 *  Created on: Nov 25, 2014
 *      Author: netmind
 */

#include "CaHttpUrlParser.h"
namespace cahttp {
#define CHECK_LEN_END(POS, LEN) if(POS>=LEN) {_url_errorno=100;goto __PARSE_END;}
#define WALK_SP(POS, LEN, BUF) for(;POS<LEN && BUF[POS]==' ';POS++)
#define WALK_UNTIL(POS, LEN, BUF, DELC) for(;POS<LEN && BUF[POS]!=DELC;POS++)
#define WALK_UNTIL2(POS, LEN, BUF, DELI1, DELI2) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 ;POS++)
#define WALK_UNTIL3(POS, LEN, BUF, DELI1, DELI2, DELI3) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 && BUF[POS]!=DELI3;POS++)
#define WALK_UNTIL4(POS, LEN, BUF, DELI1, DELI2, DELI3, DELI4) for(;POS<LEN && BUF[POS]!=DELI1 && BUF[POS]!=DELI2 && BUF[POS]!=DELI3 && BUF[POS]!=DELI4;POS++)
#define CHECK_REMAIN_END(POS, LEN, REQ_LEN) if(LEN-POS < REQ_LEN) {_url_errorno=100; goto __PARSE_END; }
#define WALK_CHAR(POS, BUF, DELI) if(BUF[POS++] != DELI) goto __PARSE_END

using namespace std;

int __kv_callback_map(void* list, string k, string v);
int __kv_callback_vec(void* list, string k, string v);

CaHttpUrlParser::CaHttpUrlParser() {
}

CaHttpUrlParser::~CaHttpUrlParser() {
}

CaHttpUrlParser::CaHttpUrlParser(CaHttpUrlParser&& other) {
	*this = move(other);
}

CaHttpUrlParser& CaHttpUrlParser::operator=(CaHttpUrlParser &&other) {
	if (this != &other) {
//			mRawUrl = move(other.mRawUrl);
		scheme = move(other.scheme);
		userName = move(other.userName);
		hostName = move(other.hostName);
		port = move(other.port);
		path = move(other.path);
		query = move(other.query);
		fragment = move(other.fragment);
	}
	return *this;
}

string CaHttpUrlParser::urlDecode(string str) {
	int _url_errorno = 0;
	size_t pos = 0, per = 0;
	size_t len = str.size();
	const char* buf = str.c_str();
	string decstr;
	_url_errorno = 0;
	for (per = pos = 0;;) {
		WALK_UNTIL2(pos, len, buf, '%', '+');
		decstr.append(buf, per, pos - per);
		if (pos >= len)
			goto __PARSE_END;
		if (buf[pos] == '%') {
			CHECK_REMAIN_END(pos, len, 3);
			try {
				char c = CaHttpUrlParser::toChar(buf + pos + 1);
				decstr.push_back(c);
				pos += 3;
				per = pos;
			} catch (int err) {
				_url_errorno = err;
				goto __PARSE_END;
			}
			if (pos >= len)
				goto __PARSE_END;
		}
		else if (buf[pos] == '+') {
			decstr.push_back(' ');
			pos++;
			per = pos;
		}
	}
	__PARSE_END: if (_url_errorno != 0)
		return "";
	return decstr;

}

string CaHttpUrlParser::urlEncode(string s) {
	const char *ptr = s.c_str();
	string enc;
	char c;
	char phex[3] = { '%' };
	for (size_t i = 0; i < s.size(); i++) {
		c = ptr[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '*' || c == '.') {
			enc.push_back(c);
		}
		else if (c == ' ') {
			enc.push_back('+');
		}
		else {
			toHex(phex + 1, c);
			enc.append(phex, 0, 3);
		}
	}
	return enc;
}

void CaHttpUrlParser::toHex(char* desthex, char c) {
	static char hextable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	desthex[0] = hextable[c >> 4];
	desthex[1] = hextable[c & 0x0f];
}

int CaHttpUrlParser::parsePath(vector<string>* folders, string pathstr) {
	int _url_errorno = 0;
	int path_pos = 0;
	size_t pos = 0;
	size_t len = pathstr.size();
	const char* str = pathstr.c_str();
	string name;
	for (pos = 0;;) {
		WALK_CHAR(pos, str, '/');
		path_pos = pos;
		CHECK_LEN_END(pos, len);
		WALK_UNTIL(pos, len, str, '/');
		name = pathstr.substr(path_pos, pos - path_pos);
		folders->push_back(name);
	}
	__PARSE_END: return folders->size();
}

int CaHttpUrlParser::parse(const char* str, size_t len) {
	int _url_errorno = 0;
	size_t pos;
	int scheme_pos, host_pos, port_pos, path_pos, param_pos, tag_pos;
	pos = 0;
	WALK_SP(pos, len, str); // remove preceding spaces.
	if (str[pos] == '/') {
		goto __PARSE_PATH;
	}

	// start protocol scheme
	scheme_pos = pos;
	WALK_UNTIL(pos, len, str, ':');
	CHECK_LEN_END(pos, len);
	scheme.assign(str + scheme_pos, pos - scheme_pos);
	CHECK_REMAIN_END(pos, len, 3);
	WALK_CHAR(pos, str, ':');
	WALK_CHAR(pos, str, '/');

	// start host address
	//__PARSE_HOST:
	WALK_CHAR(pos, str, '/');
	host_pos = pos;
	WALK_UNTIL4(pos, len, str, ':', '/', '?', '@');
	if(pos<len && str[pos]=='@'){
		userName.assign(str+host_pos, pos-host_pos);
		pos++;
		host_pos = pos;
		WALK_UNTIL3(pos, len, str, ':', '/', '?');
	}

	if (pos < len) {
		hostName.assign(str + host_pos, pos - host_pos);
		if (str[pos] == ':')
			goto __PARSE_PORT;
		if (str[pos] == '/')
			goto __PARSE_PATH;
		if (str[pos] == '?')
			goto __PARSE_PARAM;
	}
	else {
		hostName.assign(str + host_pos, pos - host_pos);
	}

	__PARSE_PORT:
	WALK_CHAR(pos, str, ':');
	port_pos = pos;
	WALK_UNTIL2(pos, len, str, '/', '?');
	port.assign(str + port_pos, pos - port_pos);
	CHECK_LEN_END(pos, len);
	if (str[pos] == '?')
		goto __PARSE_PARAM;
	__PARSE_PATH: path_pos = pos;
	WALK_UNTIL(pos, len, str, '?');
	path.assign(str + path_pos, pos - path_pos);
	CHECK_LEN_END(pos, len);
	__PARSE_PARAM:
	WALK_CHAR(pos, str, '?');
	param_pos = pos;
	WALK_UNTIL(pos, len, str, '#');
	query.assign(str + param_pos, pos - param_pos);
	CHECK_LEN_END(pos, len);

	// start parsing fragment
	WALK_CHAR(pos, str, '#');
	tag_pos = pos;
	fragment.assign(str + tag_pos, len - tag_pos);
	__PARSE_END: return 0;
}

unique_ptr<CaHttpUrlParser> CaHttpUrlParser::newUrl(const string &urlstr) {
	unique_ptr<CaHttpUrlParser> url(new CaHttpUrlParser);
//	url->mRawUrl = urlstr;
//	url->parse();
	url->parse(urlstr.data(), urlstr.size());
	return url;
}

char CaHttpUrlParser::toChar(const char* hex) {
	unsigned char nible[2];
	unsigned char c, base;
	for (int i = 0; i < 2; i++) {
		c = hex[i];
		if (c >= '0' && c <= '9') {
			base = '0';
		}
		else if (c >= 'A' && c <= 'F') {
			base = 'A' - 10;
		}
		else if (c >= 'a' && c <= 'f') {
			base = 'a' - 10;
		}
		else {
			throw 200;
		}
		nible[i] = c - base;
	}
	return ((nible[0] << 4) | nible[1]);
}

size_t CaHttpUrlParser::parseKeyValueMap(unordered_map<string, string> *kvmap, string rawstr, bool strict) {
	return parseKeyValue(rawstr, __kv_callback_map, kvmap, strict);
}

size_t CaHttpUrlParser::parseKeyValueList(vector<query_kv_t> *kvvec, string rawstr, bool strict) {
	return parseKeyValue(rawstr, __kv_callback_vec, kvvec, strict);
}

size_t CaHttpUrlParser::parseKeyValue(string rawstr, __kv_callback kvcb, void* obj, bool strict) {

	int _url_errorno = 0;
	const char *str = rawstr.c_str();
	size_t pos, len, item_len;
	pos = 0;
	len = rawstr.size();

	string key, val;
	size_t key_pos;
	WALK_SP(pos, len, str);
	CHECK_LEN_END(pos, len);
	key_pos = pos;
	item_len = 0;
	for (;;) {
		WALK_UNTIL2(pos, len, str, '=', '&');
		if (pos >= len || str[pos] == '&') {
			// Be careful for boundary check error to be caused. !!!
			// *** Do not access str[] any more in this block. !!!

			val = rawstr.substr(key_pos, pos - key_pos);

			if (strict == true) {
				if (key.empty() == false && val.empty() == false) {
					kvcb(obj, key, val);
					item_len++;
				}
			}
			else if (!(key.empty() == true && val.empty() == true)) {
				kvcb(obj, key, val);
				item_len++;
			}

			key.clear();
			val.clear();
			if (pos >= len)
				goto __PARSE_END;
			pos++;
			key_pos = pos;
		}
		else if (str[pos] == '=') {
			key = rawstr.substr(key_pos, pos - key_pos);
			pos++;
			key_pos = pos;
		}
	}
	__PARSE_END: if (_url_errorno != 0)
		return -1;
	return item_len;
}

int __kv_callback_map(void* list, string k, string v) {
	auto *map = (unordered_map<string, string>*) list;
	(*map)[k] = v;
	return map->size();
}

int __kv_callback_vec(void* list, string k, string v) {
	auto *vec = (vector<query_kv_t>*) list;
	query_kv_t t = { k, v };
	vec->push_back(t);
	return vec->size();
}

string CaHttpUrlParser::getRelativePath(string path, string prefix) {
	if (prefix.size() <= path.size())
		return path.substr(prefix.size());
	else
		return "";
}

string CaHttpUrlParser::dump() {
	string ds;
	ds = "scheme: " + scheme + "\n";
	ds += "user: " + userName + "\n";
	ds += "host: " + hostName + "\n";
	ds += "port: " + port + "\n";
	ds += "path: " + path + "\n";
	ds += "query: " + query + "\n";
	ds += "fragment: " + fragment + "\n";
	return move(ds);
}

int CaHttpUrlParser::parse(const string& s) {
	return parse(s.data(), s.size());
}

void CaHttpUrlParser::clear() {
	scheme = "";
	userName = "";
	hostName = "";
	port = "";
	path = "";
	query = "";
	fragment = "";
}
}
