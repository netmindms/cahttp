/*
 * SDPParser.h
 *
 *  Created on: Oct 9, 2015
 *      Author: netmind
 */

#ifndef CAHTTP_SDPPARSER_H_
#define CAHTTP_SDPPARSER_H_
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <utility>


class SDPParser {
public:
	enum error_e {
		NONE, T_ERROR, O_ERROR, UNKNOWN,
	};
	SDPParser();
	virtual ~SDPParser();
	size_t parse(const char* data, size_t len);
	size_t parse(const std::string& data);
	int mVer;
	std::string ipstr;
	std::string nettype;
	std::string addrtype;
	std::string bufs;
	std::string sessionName;

	typedef std::pair<std::string, std::string> attr_t;
	struct origin_t {
		std::string userName;
		std::string sessId;
		int sessVer;
		std::string nettype; // IN
		std::string addrtype; // IPV4, IPV6
		std::string addr;
	};
	struct media_t {
		int port;
		int port_num;
		std::string media;
		std::string proto;
		std::vector<int> formats;
		std::vector<attr_t> attrs;
	};

	std::vector<media_t>& getAllMedias();
	media_t* getMedia(const char* media);
	attr_t* getAttr(media_t &media, const char* attr_name);
	std::vector<attr_t>& getAllAttr(media_t &media);
	const std::string& getMediaAttr(const char* media, const char* attr_name, const std::string &def);
private:

	size_t parse_v(const char* ptr, size_t len);
	size_t parse_o(const char* ptr, size_t len);
	size_t parse_m(const char* ptr, size_t len, media_t &media);
	size_t parse_a(const char* ptr, size_t len, attr_t &attr);
	size_t parse_c(const char* ptr, size_t len);
	size_t parse_t(const char* ptr, size_t len);
	size_t parse_simple_line(const char* ptr, size_t len, char sc,
			std::string &line);

	error_e mError;
	int ttl;
	std::string tbuf;
	uint64_t startTime, endTime;
	origin_t origin;
	std::vector<media_t> mMedias;
	std::vector<attr_t> mAttrs;
	std::string mSessInfo;

};

#endif /* CAHTTP_SDPPARSER_H_ */
