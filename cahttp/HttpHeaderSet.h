/*
 * HttpHeaderSet.h
 *
 *  Created on: Jul 29, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPHEADERSET_H_
#define SRC_HTTPHEADERSET_H_
#include <memory>
#include <vector>

#include "CaHttpCommon.h"
namespace cahttp {
class HttpHeaderSet {
public:
	HttpHeaderSet();
	HttpHeaderSet(HttpHeaderSet &&other) { // move constructor
		*this = std::move(other);
	}

	HttpHeaderSet& operator=(HttpHeaderSet &&other) { // move operator
		if (&other != this) {
			mHdrs = std::move(other.mHdrs);
		}
		return *this;
	}
	virtual ~HttpHeaderSet();

	void setContentLength(size_t len);
	void setContentType(const char* type);
	void setContentType(std::string &&type);

	void addHdr(std::string &&name, std::string &&val);
	void addHdr(const std::string &name, const std::string &val);
	void addHdrVals(std::string &&name, std::vector<std::string> &&vals);
	void setHdr(std::string &&name, std::string &&val, bool replace=true);
	void setHdr(const std::string &name, const std::string &val, bool replace=true);
	void setHdrVals(std::string &&name, std::vector<std::string> &&vals, bool replace=true);
	const hdrpair* findSet(const char* name);
	std::vector<hdrpair> fetchHdrSet();
	void clear();
	std::string dump();
private:
	std::vector<hdrpair> mHdrs;
	hdrpair* findHeader(const char* name);
};

typedef std::unique_ptr<HttpHeaderSet> upHttpHeaderSet;
}
#endif /* SRC_HTTPHEADERSET_H_ */
