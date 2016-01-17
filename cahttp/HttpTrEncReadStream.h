/*
 * HttpTrEncReadStream.h
 *
 *  Created on: Oct 20, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPTRENCREADSTREAM_H_
#define SRC_HTTPTRENCREADSTREAM_H_
#include "HttpBaseReadStream.h"
namespace cahttp {
class HttpTrEncReadStream : public HttpBaseReadStream {
public:
	HttpTrEncReadStream();
	virtual ~HttpTrEncReadStream();
	virtual std::pair<const char*, int64_t> getDataPtr() override ;
	virtual size_t remain() override ;
	virtual void consume(size_t len) override ;
	virtual ssize_t store(const char* ptr, size_t len) override {
		return -1;
	}
	void putData(const char* ptr, size_t len);
private:
	char* mBuf;
	int mBufSize;
	size_t mRi;
	size_t mWi;
	size_t mMaxBuf;

};
}
#endif /* SRC_HTTPTRENCREADSTREAM_H_ */
