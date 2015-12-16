/*
 * HttpHeaderSet.cpp
 *
 *  Created on: Jul 29, 2015
 *      Author: netmind
 */

#include "HttpHeaderSet.h"
namespace cahttp {
HttpHeaderSet::HttpHeaderSet() {

}

HttpHeaderSet::~HttpHeaderSet() {
}

void HttpHeaderSet::addHdr(string&& name, string&& val) {
	mHdrs.emplace_back(hdrpair(move(name), { move(val) }));
}

void HttpHeaderSet::addHdr(const string& name, const string& val) {
	addHdr(string(name), string(val));
}

void HttpHeaderSet::addHdrVals(string&& name, vector<string> && vals) {
	mHdrs.emplace_back(hdrpair(move(name), move(vals)));
}

void HttpHeaderSet::setHdr(string&& name, string&& val, bool replace) {
	auto *ph = findHeader(name.data());
	if(ph) {
		if(replace) {
			ph->second = {move(val)};
		}
		else {
			ph->second.emplace_back(move(val));
		}
	}
	else {
		addHdr(move(name), move(val));
	}
}

void HttpHeaderSet::setHdrVals(string&& name, vector<string> && vals, bool replace) {
	auto phdr = findHeader(name.data());
	if (phdr) {
		if(replace) {
			phdr->second = move(vals);
		}
		else {
			for(auto &s: vals) {
				phdr->second.emplace_back(move(s));
			}
		}
	}
	else {
		addHdrVals(move(name), move(vals));
	}
}

const hdrpair* HttpHeaderSet::findSet(const char *name) {
	return findHeader(name);
}

std::vector<hdrpair> HttpHeaderSet::fetchHdrSet() {
	return move(mHdrs);
}

void HttpHeaderSet::setHdr(const string& name, const string& val, bool replace) {
	setHdr(string(name), string(val), replace);
}

void HttpHeaderSet::clear() {
	mHdrs.clear();
}

void HttpHeaderSet::setContentLength(size_t len) {
	setHdr("Content-Length", to_string(len));
}

void HttpHeaderSet::setContentType(const char* type) {
	setContentType(string(type));
}

void HttpHeaderSet::setContentType(string&& type) {
	setHdr("Content-Type", move(type));
}

hdrpair* HttpHeaderSet::findHeader(const char* name) {
	for (auto &h : mHdrs) {
		if (h.first == name) {
			return &h;
		}
	}
	return nullptr;
}

string HttpHeaderSet::dump() {
	string ds;
	for (auto &h : mHdrs) {
		ds += h.first + ": ";
		for (auto &v : h.second) {
			ds += v + ", ";
		}
		ds.pop_back();
		ds.pop_back();
		ds += "\n";
	}
	return move(ds);
}
}
