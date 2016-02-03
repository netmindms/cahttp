/*
 * HttpFileReadStream.h
 *
 *  Created on: Jul 28, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPFILEREADSTREAM_H_
#define SRC_HTTPFILEREADSTREAM_H_
#include "HttpBaseReadStream.h"
#include <ednio/EdFile.h>
using namespace edft;
namespace cahttp {
class HttpFileReadStream : public HttpBaseReadStream {
public:
	HttpFileReadStream();
	virtual ~HttpFileReadStream();
//	virtual std::string readStr(size_t len) override ;
	std::pair<const char*, int64_t> getDataPtr() override;
	ssize_t store(const char* ptr, size_t len) override;
	void consume(size_t) override;
	virtual size_t remain() override;
	int open(const char* path, size_t chksize=4*1024);
	void close();
	void setRange(size_t offset, size_t len);
	size_t getFileSize() const { return mFileSize; }

private:
	EdFile mFile;
	size_t mFileSize;
	size_t mDataSize;
	size_t mReadCnt;
	char *mBuf;
	size_t mChunkRemain;
	size_t mBufSize;
	size_t mChunkReadPos;
};

typedef std::unique_ptr<HttpFileReadStream> upHttpFileReadStream;
}
#endif /* SRC_HTTPFILEREADSTREAM_H_ */
