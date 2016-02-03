/*
 * CaRtspMsg.cpp
 *
 *  Created on: Oct 5, 2015
 *      Author: root
 */
#define LOG_LEVEL LOG_DEBUG


#include <climits>
#include "flog.h"
#include "CaHttpBaseMsg.h"
#include "CaHttpCommon.h"
using namespace std;

namespace cahttp {
static string _NULL_HDR;

CaHttpBaseMsg::CaHttpBaseMsg()
{
	msgType = 0;
	method = HTTP_GET;
	mContentLen = CLEN_INVALID; // not set
	mProtocol = MSG_PROTOCOL::PRT_HTTP11;
}

CaHttpBaseMsg::CaHttpBaseMsg(const CaHttpBaseMsg &other) { // TODO
	msgType = 0;
	method = HTTP_GET;
	mContentLen = CLEN_INVALID; // not set
	mProtocol = MSG_PROTOCOL::PRT_HTTP11;
}

CaHttpBaseMsg::~CaHttpBaseMsg()
{
	// TODO Auto-generated destructor stub
}

void CaHttpBaseMsg::addHdr(const char* name, const char* val) {
	addHdr(string(name), string(val));
}

void CaHttpBaseMsg::addHdr(string&& name, string&& val) {
	mHdrs.emplace_back(move(name), move(val));
}

void CaHttpBaseMsg::setHdr(const char* name, string&& val) {
	for (auto &hv : mHdrs) {
		if (hv.first == name) {
			hv.second = move(val);
			return;
		}
	}

	addHdr(string(name), move(val));
}

const char* CaHttpBaseMsg::getMethodStr() {
	return http_method_str(method);
}

void CaHttpBaseMsg::setRequest(http_method m) {
	method = m;
	msgType = 1;
}

void CaHttpBaseMsg::setResponse(int code) {
	responseCode = code;
	msgType = 2;
}

const string& CaHttpBaseMsg::getHdr(const string &name) {
	for (auto &hv : mHdrs) {
		if (hv.first == name) {
			return hv.second;
		}
	}
	return _NULL_HDR;
}

void CaHttpBaseMsg::setData(string&& data) {
	mData = move(data);
}

string CaHttpBaseMsg::fetchData() {
	string rs = move(mData);
	mData.clear();
	return move(rs);
}

const string& CaHttpBaseMsg::getData() {
	return mData;
}

int64_t CaHttpBaseMsg::getContentLen()
{
	return mContentLen;
}

void CaHttpBaseMsg::setContentLen(int64_t size) {
	if (size >= 0) {
		mContentLen = size;
		if (mContentLen == CLEN_INVALID) {
			addHdr(string("Content-Length"), to_string(size));
		}
		else {
			setHdr("Content-Length", to_string(size));
		}
	}
}

void CaHttpBaseMsg::setUrl(const char *urlstr, size_t len) {
	mUrlStr.assign(urlstr, len);
}

string CaHttpBaseMsg::serialize(bool body) {
	string encstr;
	if (msgType == 1) {
		encstr = string(http_method_str(method)) + " " + mUrlStr + " " + http_get_protocol_str(mProtocol) + "\r\n";
		ald("request line: %s", encstr);
	}
	else if (msgType == 2) {
		encstr = string(http_get_protocol_str(mProtocol)) + " " + to_string(responseCode) + " " + get_status_desc(responseCode) + "\r\n";
	}

	// header enc
	for (auto &h : mHdrs) {
		encstr += h.first + ": " + h.second + "\r\n";
	}
	encstr += "\r\n";
	if(body) {
		encstr += mData;
	}
	return move(encstr);
}

void CaHttpBaseMsg::setProtocol(MSG_PROTOCOL prt) {
	mProtocol = prt;
}

void CaHttpBaseMsg::clear() {
	msgType = 0;
	mContentLen = CLEN_INVALID;
	method = HTTP_GET;
	mHdrs.clear();
	mData.clear();
	mUrlStr.clear();
}

void CaHttpBaseMsg::setUrl(string&& urlstr) {
	mUrlParser.parse(urlstr);
	mUrlStr = move(urlstr);
}

const string& CaHttpBaseMsg::getUrl() {
	return mUrlStr;
}

const string& CaHttpBaseMsg::getUrlPath() {
	return mUrlParser.path;
}

const CaHttpUrlParser& CaHttpBaseMsg::getUrlParser() {
	return mUrlParser;
}

string CaHttpBaseMsg::dumpHdr()
{
	string ds;
	for (auto &h : mHdrs)
	{
		ds += h.first + ": " + h.second + "\n";
	}
	return move(ds);
}
}
