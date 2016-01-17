/*
 * EdUrlParser.h
 *
 *  Created on: Nov 25, 2014
 *      Author: netmind
 */

#ifndef CAHTTPURLPARSER_H_
#define CAHTTPURLPARSER_H_

#include <memory>
#include <unordered_map>
#include <tuple>
#include <vector>
#include <string>

namespace cahttp {
typedef struct {
	std::string key;
	std::string val;
} query_kv_t;

typedef int (*__kv_callback)(void* list, std::string k, std::string v);

class CaHttpUrlParser {
public:
	CaHttpUrlParser();
	CaHttpUrlParser(CaHttpUrlParser&& other);
	CaHttpUrlParser& operator=(CaHttpUrlParser &&other);
	virtual ~CaHttpUrlParser();
	static std::unique_ptr<CaHttpUrlParser> newUrl(const std::string &urlstr);
	static int parsePath(std::vector<std::string> *pdirlist, std::string pathstr);
	static std::string urlDecode(std::string str);
	static char toChar(const char* hex);
	static std::string urlEncode(std::string s);
	static void toHex(char *desthex, char c);
	static size_t parseKeyValueMap(std::unordered_map<std::string, std::string> *kvmap, std::string str, bool strict = true);
	static size_t parseKeyValueList(std::vector<query_kv_t> *kvmap, std::string rawstr, bool strict = true);
	static size_t parseKeyValue(std::string rawstr, __kv_callback kvcb, void* obj, bool strict);
	static std::string getRelativePath(std::string path, std::string prefix);
//	void parse();
	int parse(const char* s, size_t len);
	int parse(const std::string &s);
	void clear();
	std::string dump();

//	string mRawUrl;
public:
	std::string scheme;
	std::string userName;
	std::string hostName;
	std::string port;
	std::string path;
	std::string query;
	std::string fragment;
};

typedef std::unique_ptr<CaHttpUrlParser> upUrlParser;
}
#endif /* CAHTTPURLPARSER_H_ */
