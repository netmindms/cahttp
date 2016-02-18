/*
 * CaHttpCommon.h
 *
 *  Created on: Apr 20, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPCOMMON_H_
#define SRC_CAHTTPCOMMON_H_
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <ednio/EdNio.h>


namespace cahttp {

#if (__GNUC__==4 && __GNUC_MINOR__>=9)
#define CAHTTP_REGEX_URLPATTERN
#endif


typedef edft::EdObject CaUserObj;
typedef std::unique_ptr<CaUserObj> upCaUserObj;

namespace CAS {
	// constant header name string
	extern const std::string HS_CONTENT_LEN;
	extern const std::string HS_CONTENT_TYPE;
	extern const std::string HS_DATE;
	extern const std::string HS_SERVER;
	extern const std::string HS_USER_AGENT;
	extern const std::string HS_CSEQ;
	extern const std::string HS_ACCEPT;
	extern const std::string HS_TRANSFER_ENC;
	extern const std::string HS_HOST;

	// constant method string
	extern const std::string MS_GET;
	extern const std::string MS_POST;
	extern const std::string MS_PUT;
	extern const std::string MS_DELETE;

	// constant content-type string
	extern const std::string CT_TEXT_HTML;
	extern const std::string CT_TEXT_PLAIN;
	extern const std::string CT_APP_OCTET;
	extern const std::string CT_APP_JSON;
	extern const std::string CT_AUDIO_MP4;
	extern const std::string CT_VIDEO_MP4;
	extern const std::string CT_VIDEO_MPEG;
	extern const std::string CT_IMAGE_JPEG;
	extern const std::string CT_IMAGE_PNG;

}

enum {
	HDR__BEGIN,
	HDR_CONTENT_TYPE,
	HDR_CONTENT_LENGTH,
	HDR_AUTHORIZATION,
	HDR_HOST,
	HDR_DATE,
	HDR_ACCEPT,
	HDR_SERVER,
	HDR_USER_AGENT,
	HDR_CONNECTION,
	HDR_ACCEPT_ENCODING,
	HDR_ETAG,
	HDR_EXPECT,
	HDR_ALLOW,
	HDR_UPGRADE,
	HDR_WWW_AUTHENTICATE,
	HDR_ACCEPT_CHARSET,
	HDR_ACCEPT_LANGUAGE,
	HDR_AGE,
	HDR_CACHE_CONTROL,
	HDR_CONTENT_ENCODING,
	HDR_CONTENT_LANGUAGE,
	HDR_CONTENT_LOCATION,
	HDR_CONTENT_MD5,
	HDR_CONTENT_RANGE,
	HDR_EXPIRES,
	HDR_FROM,
	HDR_IF_MATCH,
	HDR_IF_MODIFIED_SINCE,
	HDR_IF_NONE_MATH,
	HDR_IF_RANGE,
	HDR_IF_UNMODIFIED_SINCE,
	HDR_LAST_MODIFIED,
	HDR_LOCATION,
	HDR_MAX_FORWARDS,
	HDR_PRAGMA,
	HDR_PROXY_AUTHENTICATE,
	HDR_PROXY_AUTHORIZATION,
	HDR_RANGE,
	HDR_REFERER,
	HDR_RETRY_AFTER,
	HDR_TE,
	HDR_TRAILER,
	HDR_TRANSFER_ENCODING,
	HDR_VARY,
	HDR_VIA,
	HDR_WARNING,

	HDR__MAX,
};

enum SEND_RESULT {
	SEND_PENDING=-1,
	SEND_OK=0,
	SEND_FAIL=1,
	SEND_NEXT=2,
};


extern std::string gHttpMethodStr[30];

#define METHOD_STR(m) gHttpMethodStr[m]

typedef std::pair<std::string, std::vector<std::string>> hdrpair;
typedef std::vector<hdrpair> HdrList;


std::vector<uint32_t> dns_lookup(const std::string& hostname);
std::string get_ipstr(uint32_t ip);

uint32_t new_handle();
const std::string& get_status_desc(int status);
void get_http_date_str(std::string &ds, const time_t *t);
void add_http_hdr(HdrList &hdrs, std::string &&name, std::string &&val);
void add_http_hdr(HdrList &hdrs, std::string &&name, std::vector<std::string> &&val);
void set_http_hdr(HdrList &hdrs, std::string &&name, std::string &&val);
uint32_t get_ip_from_hostname(const std::string& hostname);
std::string get_http_cur_date_str();

void set_log_level(int level);

}
#endif /* SRC_CAHTTPCOMMON_H_ */
