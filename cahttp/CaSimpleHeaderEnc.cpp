/*
 * CaSimpleHeaderEnc.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: root
 */

#include "CaHttpCommon.h"
#include "CaSimpleHeaderEnc.h"
namespace cahttp {
CaSimpleHeaderEnc::CaSimpleHeaderEnc() {
	// TODO Auto-generated constructor stub

}

CaSimpleHeaderEnc::~CaSimpleHeaderEnc() {
	// TODO Auto-generated destructor stub
}

void CaSimpleHeaderEnc::addHdr(const char* name, const char* val) {
	mEncStr = mEncStr+name+": "+val+"\r\n";
}

void CaSimpleHeaderEnc::addHdr(const char* name, const string& val) {
	addHdr(name, val.data());
}

void CaSimpleHeaderEnc::addHdr(const string& name, const string& val) {
	addHdr(name.data(), val.data());
}

void CaSimpleHeaderEnc::setResponse(int code, MSG_PROTOCOL prt) {
	mEncStr = string(http_get_protocol_str(prt))+" "+to_string(code)+" "+get_status_desc(code)+"\r\n";
}

void CaSimpleHeaderEnc::setRequest(http_method method, const char* urlstr, MSG_PROTOCOL prt) {
	mEncStr = string(http_method_str(method))+" "+urlstr+" "+http_get_protocol_str(prt)+"\r\n";
}


string CaSimpleHeaderEnc::encode() {
	mEncStr += "\r\n";
	return move(mEncStr);
}
}
