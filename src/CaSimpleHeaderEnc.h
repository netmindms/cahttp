/*
 * CaSimpleHeaderEnc.h
 *
 *  Created on: Oct 6, 2015
 *      Author: root
 */

#ifndef EXTERNAL_CAHTTP_CASIMPLEHEADERENC_H_
#define EXTERNAL_CAHTTP_CASIMPLEHEADERENC_H_

#include <string>

#include "http_parser.h"
namespace cahttp {
class CaSimpleHeaderEnc {
public:
	CaSimpleHeaderEnc();
	virtual ~CaSimpleHeaderEnc();
	void setResponse(int code, MSG_PROTOCOL prt);
	void setRequest(http_method method, const char* urlstr, MSG_PROTOCOL prt);
	void addHdr(const char *name, const char*val);
	void addHdr(const char *name, const std::string& val);
	void addHdr(const std::string &name, const std::string& val);
	std::string encode();
private:
	std::string mEncStr;
};
}
#endif /* EXTERNAL_CAHTTP_CASIMPLEHEADERENC_H_ */
