/*
 * CaHttpCommon.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: netmind
 */

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <mutex>
#include <array>
#include <utility>
#include "http_parser.h"
#include "CaHttpCommon.h"
#include "flog.h"

using namespace std;

// TODO:
#if 0
#define HEADER_MAP() \
	HH(CONTENT_TYPE, Content-Type),\
	HH(CONTENT_LEN, Content-Len), \
	HH(ACCEPT, Accept), \
	HH(_MAX, None), \

namespace cahttp {
#define HH(HDEF, HSTR) HDR_##HDEF
enum HTTP_HEADER {
	HEADER_MAP()
};
#undef HH

#define HH(HNAME, HSTR) #HSTR
const std::string _gHeaderStrMap[] = {
		HEADER_MAP()
};
#undef HH
}
#endif

namespace cahttp {

namespace CAS {
	const string HS_CONTENT_LEN = "Content-Length";
	const string HS_CONTENT_TYPE = "Content-Type";
	const string HS_DATE = "Date";
	const string HS_SERVER = "Server";
	const string HS_USER_AGENT = "User-Agent";
	const string HS_CSEQ = "CSeq";
	const string HS_ACCEPT = "Accept";
	const string HS_TRANSFER_ENC = "Transfer-Encoding";
	const string HS_HOST = "Host";

	const string MS_GET = "GET";
	const string MS_POST = "POST";
	const string MS_PUT = "PUT";
	const string MS_DELETE = "DELETE";

	const string CT_TEXT_HTML="text/html";
	const string CT_TEXT_PLAIN="text/plain";
	const string CT_APP_OCTET="application/octet-stream";
	const string CT_APP_JSON="application/json";
	const string CT_AUDIO_MP4="audio/mp4";
	const string CT_VIDEO_MP4 = "video/mp4";
	const string CT_VIDEO_MPEG = "video/mpeg";
	const string CT_IMAGE_JPEG = "image/jpeg";
	const string CT_IMAGE_PNG = "image/png";
}

string gHttpMethodStr[30];

array<string, HDR__MAX> gHdrDef;
static array<pair<int, string>, 41> gStatusDescMap { { { 000, "" }, { 100, "Continue" }, { 101, "Switching Protocols" },
	{ 200, "OK" }, { 201, "Created" }, { 202, "Accepted" }, { 203, "Non-Authoritative Information" },
	{ 204, "No Content" }, { 205, "Reset Content" }, { 206, "Partial Content" }, { 300, "Multiple Choices" }, { 301, "Moved Permanently" }, { 302, "Found" }, { 303, "See Other" }, { 304, "Not Modified" }, { 305, "Use Proxy" }, { 307, "Temporary Redirect" }, { 400, "Bad Request" }, { 401, "Unauthorized" }, { 402, "Payment Required" }, { 403, "Forbidden" }, { 404, "Not Found" }, { 405, "Method Not Allowed" }, { 406, "Not Acceptable" }, { 407, "Proxy Authentication Required" }, { 408, "Request Timeout" }, { 409, "Conflict" }, { 410, "Gone" }, { 411, "Length Required" }, { 412, "Precondition Failed" }, { 413, "Request Entity Too Large" }, { 414, "Request-URI Too Long" }, { 415, "Unsupported Media Type" }, { 416, "Requested Range Not Satisfiable" }, { 417, "Expectation Failed" }, { 500, "Internal Server Error" }, { 501, "Not Implemented" }, { 502, "Bad Gateway" }, { 503, "Service Unavailable" }, { 504, "Gateway Timeout" }, { 505, "HTTP Version Not Supported" } } };

class __httpinit__ {
public:
	__httpinit__() {
		gHttpMethodStr[HTTP_DELETE] = "DELETE";
		gHttpMethodStr[HTTP_GET] = "GET";
		gHttpMethodStr[HTTP_HEAD] = "HEAD";
		gHttpMethodStr[HTTP_POST] = "POST";
		gHttpMethodStr[HTTP_PUT] = "PUT";
		gHttpMethodStr[HTTP_CONNECT] = "CONNECT";
		gHttpMethodStr[HTTP_OPTIONS] = "OPTIONS";
		gHttpMethodStr[HTTP_TRACE] = "TRACE";
		gHttpMethodStr[HTTP_COPY] = "COPY";
		gHttpMethodStr[HTTP_LOCK] = "LOCK";
		gHttpMethodStr[HTTP_MKCOL] = "MKCOL";
		gHttpMethodStr[HTTP_MOVE] = "MOVE";
		gHttpMethodStr[HTTP_PROPFIND] = "PROPFIND";
		gHttpMethodStr[HTTP_PROPPATCH] = "PROPPATCH";
		gHttpMethodStr[HTTP_SEARCH] = "SEARCH";
		gHttpMethodStr[HTTP_UNLOCK] = "UNLOCK";
		gHttpMethodStr[HTTP_REPORT] = "REPORT";
		gHttpMethodStr[HTTP_MKACTIVITY] = "MKACTIVITY";
		gHttpMethodStr[HTTP_CHECKOUT] = "CHECKOUT";
		gHttpMethodStr[HTTP_MERGE] = "MERGE";
		gHttpMethodStr[HTTP_MSEARCH] = "MSEARCH";
		gHttpMethodStr[HTTP_NOTIFY] = "NOTIFY";
		gHttpMethodStr[HTTP_SUBSCRIBE] = "SUBSCRIBE";
		gHttpMethodStr[HTTP_UNSUBSCRIBE] = "UNSUBSCRIBE";
		gHttpMethodStr[HTTP_PATCH] = "PATCH";
		gHttpMethodStr[HTTP_PURGE] = "PURGE";

		// header definition string
		gHdrDef[HDR_ACCEPT] = "accept";
		gHdrDef[HDR_ACCEPT_CHARSET] = "accept-charset";
		gHdrDef[HDR_ACCEPT_ENCODING] = "accept-encoding";
		gHdrDef[HDR_ACCEPT_LANGUAGE] = "accept-language";
		gHdrDef[HDR_AGE] = "age";
		gHdrDef[HDR_ALLOW] = "allow";
		gHdrDef[HDR_AUTHORIZATION] = "authorization";
		gHdrDef[HDR_CACHE_CONTROL] = "cache-control";
		gHdrDef[HDR_CONNECTION] = "connection";
		gHdrDef[HDR_CONTENT_ENCODING] = "content-encoding";
		gHdrDef[HDR_CONTENT_LANGUAGE] = "content-language";
		gHdrDef[HDR_CONTENT_LENGTH] = "content-length";
		gHdrDef[HDR_CONTENT_MD5] = "content-md5";
		gHdrDef[HDR_CONTENT_RANGE] = "content-range";
		gHdrDef[HDR_CONTENT_TYPE] = "content-type";
		gHdrDef[HDR_DATE] = "date";
		gHdrDef[HDR_ETAG] = "etag";
		gHdrDef[HDR_EXPECT] = "expect";
		gHdrDef[HDR_EXPIRES] = "expires";
		gHdrDef[HDR_FROM] = "from";
		gHdrDef[HDR_HOST] = "host";
		gHdrDef[HDR_IF_MATCH] = "if-match";
		gHdrDef[HDR_IF_MODIFIED_SINCE] = "if-modified-since";
		gHdrDef[HDR_IF_NONE_MATH] = "if-none-match";
		gHdrDef[HDR_IF_RANGE] = "if-range";
		gHdrDef[HDR_IF_UNMODIFIED_SINCE] = "if-unmodified-since";
		gHdrDef[HDR_LAST_MODIFIED] = "last-modified";
		gHdrDef[HDR_LOCATION] = "location";
		gHdrDef[HDR_MAX_FORWARDS] = "max-forwards";
		gHdrDef[HDR_PRAGMA] = "pragma";
		gHdrDef[HDR_PROXY_AUTHENTICATE] = "proxy-authenticate";
		gHdrDef[HDR_PROXY_AUTHORIZATION] = "proxy-authorization";
		gHdrDef[HDR_RANGE] = "range";
		gHdrDef[HDR_REFERER] = "referer";
		gHdrDef[HDR_RETRY_AFTER] = "retry-after";
		gHdrDef[HDR_SERVER] = "server";
		gHdrDef[HDR_TE] = "te";
		gHdrDef[HDR_TRAILER] = "trailer";
		gHdrDef[HDR_TRANSFER_ENCODING] = "transfer-encoding";
		gHdrDef[HDR_UPGRADE] = "upgrade";
		gHdrDef[HDR_USER_AGENT] = "user-agent";
		gHdrDef[HDR_VARY] = "vary";
		gHdrDef[HDR_VIA] = "via";
		gHdrDef[HDR_WARNING] = "warning";
		gHdrDef[HDR_WWW_AUTHENTICATE] = "www-authenticate";

	}

};

__httpinit__ _ghttplibinst;

const char* __HTTP_ERR_STR[] = {
#define __ERRDEF(id, name) #name,
		__HTTP_ERR_MAP(__ERRDEF)
};
#undef __ERRDEF

vector<uint32_t> dns_lookup(const string& hostname) {
	vector<uint32_t> result;
	int i;
	struct hostent *he;
	struct in_addr **addr_list;

	if ((he = gethostbyname(hostname.data())) == NULL) {
		return move(result);
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	for (i = 0; addr_list[i] != NULL; i++) {
//		printf("%s ", inet_ntoa(*addr_list[i]));
		result.emplace_back(addr_list[i]->s_addr);
	}
	return move(result);
}

string get_ipstr(uint32_t ip) {
	in_addr in;
	in.s_addr = ip;
	return string(inet_ntoa(in));
}

uint32_t new_handle() {
	static uint32_t seed = 0;
	static mutex mtx;
	uint32_t handle;
	mtx.lock();
	handle = ++seed;
	if (handle == 0)
		handle = ++seed;
	mtx.unlock();
	return handle;
}

const string& get_status_desc(int status) {
	for (auto &desc : gStatusDescMap) {
		if (status == desc.first) {
			return desc.second;
		}
	}
	return gStatusDescMap[0].second;
}

static char _monthString[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char _dayString[][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void get_httpDate(char* buf) {
	time_t t;
	struct tm tmdata;
	time(&t);
	gmtime_r(&t, &tmdata);
	//Tue, 08 Jul 2014 00:09:47 GMT
	sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT", _dayString[tmdata.tm_wday], tmdata.tm_mday, _monthString[tmdata.tm_mon], 1900 + tmdata.tm_year, tmdata.tm_hour, tmdata.tm_min, tmdata.tm_sec);
}

void get_http_date_str(string &ds, const time_t *t) {
	struct tm tmdata;
	gmtime_r(t, &tmdata);
	char buf[50];
	sprintf(buf, "%s, %02d %s %d %02d:%02d:%02d GMT", _dayString[tmdata.tm_wday], tmdata.tm_mday, _monthString[tmdata.tm_mon], 1900 + tmdata.tm_year, tmdata.tm_hour, tmdata.tm_min, tmdata.tm_sec);
	ds = buf;
}

void add_http_hdr(HdrList &hdrs, string &&name, string &&val) {
	hdrs.emplace_back(hdrpair(name, { val }));
}

void add_http_hdr(HdrList &hdrs, string &&name, vector<string> &&val) {
	hdrs.emplace_back(hdrpair(name, move(val)));
}

void set_http_hdr(HdrList& hdrs, string&& name, string&& val) {
	hdrpair* ph = nullptr;
	for (auto &h : hdrs) {
		if (h.first == name) {
			ph = &h;
		}
	}
	if (ph == nullptr) {
		add_http_hdr(hdrs, move(name), move(val));
	} else {
		ph->first = move(name);
		ph->second.emplace_back(move(val));
	}
}



uint32_t get_ip_from_hostname(const string& hostname) {
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ((he = gethostbyname(hostname.data())) == NULL) {
		return 0;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for (i = 0; addr_list[i] != NULL; i++) {
		return (*addr_list[i]).s_addr;
	}

	return 0;
}

string get_http_cur_date_str() {
	string ds;
	time_t t;
	time(&t);
	get_http_date_str(ds, &t);
	return move(ds);
}

void set_log_level(int level) {
	NMDU_SET_LOG_LEVEL(level);
}


const char* cahttp_err_str(ERR e) {
	if(e<sizeof(__HTTP_ERR_STR)/sizeof(__HTTP_ERR_STR[0]) )
		return __HTTP_ERR_STR[e];
	else
		return "";
}
}
