/*
 * HttpFileWriterStream.h
 *
 *  Created on: Jul 28, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPFILEWRITESTREAM_H_
#define SRC_HTTPFILEWRITESTREAM_H_
#include <ednio/EdNio.h>
#include <ednio/EdFile.h>
#include "HttpBaseWriteStream.h"

using namespace edft;
namespace cahttp {
class HttpFileWriteStream : public HttpBaseWriteStream {
public:
	HttpFileWriteStream();
	virtual ~HttpFileWriteStream();
	ssize_t write(const char* buf, size_t len) override;
	size_t size() override;
	void end() override;
	int open(const char* path);
private:
	EdFile mFile;
	size_t mSize;
};
}
#endif /* SRC_HTTPFILEWRITESTREAM_H_ */
