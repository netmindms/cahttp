/*
 * CaHttpSvrReq.h
 *
 *  Created on: Jul 21, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPSVRREQ_H_
#define SRC_CAHTTPSVRREQ_H_
#include <memory>

#include "CaHttpMsg.h"
#include "http_parser.h"
#include "HttpBaseReadStream.h"
#include "HttpBaseWriteStream.h"
namespace cahttp {
class CaHttpSvrReq {
	friend class ServCnn;
	friend class CaHttpUrlCtrl;
public:
	CaHttpSvrReq();
	virtual ~CaHttpSvrReq();
	http_method getMethod();
	HttpBaseWriteStream* getReqWriteStream();
	const CaHttpMsg& getReqMsg() const;
private:
	CaHttpMsg mReqMsg;
//	unique_ptr<HttpBaseReadStream> mRespHdrStream;
	unique_ptr<HttpBaseWriteStream> mReqWriteStream;
//	std::string mStrBuf;

	void setReqMsg(CaHttpMsg &&msg);
	void buildBaseRespMsg(CaHttpMsg &msg, int status);
};
}
#endif /* SRC_CAHTTPSVRREQ_H_ */
