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
		*this = move(other);
	}

	HttpHeaderSet& operator=(HttpHeaderSet &&other) { // move operator
		if (&other != this) {
			mHdrs = move(other.mHdrs);
		}
		return *this;
	}
	virtual ~HttpHeaderSet();

	void setContentLength(size_t len);
	void setContentType(const char* type);
	void setContentType(string &&type);

	void addHdr(string &&name, string &&val);
	void addHdr(const string &name, const string &val);
	void addHdrVals(string &&name, vector<string> &&vals);
	void setHdr(string &&name, string &&val, bool replace=true);
	void setHdr(const string &name, const string &val, bool replace=true);
	void setHdrVals(string &&name, vector<string> &&vals, bool replace=true);
	const hdrpair* findSet(const char* name);
	std::vector<hdrpair> fetchHdrSet();
	void clear();
	string dump();
private:
	std::vector<hdrpair> mHdrs;
	hdrpair* findHeader(const char* name);
};

typedef unique_ptr<HttpHeaderSet> upHttpHeaderSet;
}
#endif /* SRC_HTTPHEADERSET_H_ */
