/*
 * HttpMsgStream.h
 *
 *  Created on: Jul 27, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPRESPSTREAM_H_
#define SRC_HTTPRESPSTREAM_H_
#include <bitset>
#include "HttpBaseReadStream.h"
#include "HttpStringReadStream.h"
#include "CaHttpMsg.h"
namespace cahttp {
class HttpMsgStream : public HttpBaseReadStream {
public:
	HttpMsgStream();
	virtual ~HttpMsgStream();
	void setData(std::string &&data);
	void setChunkSize(size_t size);
	void setMsg(CaHttpMsg &msg, HttpBaseReadStream* datastrm, bool tec=false);
	void setStreams(upHttpStringReadStream hdrstrm, HttpBaseReadStream *datastrm);
	void setDataStream(HttpBaseReadStream *datastrm);
	std::pair<const char*, int64_t> getDataPtr() override;
	void consume(size_t len) override;
	virtual size_t remain() override ;
	ssize_t store(const char* ptr, size_t len) override {
		return -1;;
	}

private:
	HttpStringReadStream mHdrStrm;
	HttpBaseReadStream *mDataStrm;
	upHttpBaseReadStream mDefDataStrm;
	std::string mTecPartBuf;
	size_t mChunkSize;
	int64_t mContentLen;
	int64_t mReadDataCnt;
	std::bitset<8> mFlag;
};

typedef std::unique_ptr<HttpMsgStream> upHttpMsgStream;
}
#endif /* SRC_HTTPRESPSTREAM_H_ */
