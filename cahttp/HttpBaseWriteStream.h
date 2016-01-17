/*
 * HttpDataWriteStream.h
 *
 *  Created on: Jul 24, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPBASEWRITESTREAM_H_
#define SRC_HTTPBASEWRITESTREAM_H_
#include <stdio.h>
#include <memory>
namespace cahttp {
class HttpBaseWriteStream {
public:
	HttpBaseWriteStream() {};
	virtual ~HttpBaseWriteStream() {};
	virtual ssize_t write(const char* buf, size_t len)=0;
	virtual size_t size()=0;
	virtual void end()=0;
};

typedef std::unique_ptr<HttpBaseWriteStream> upHttpBaseWriteStream;
}
#endif /* SRC_HTTPBASEWRITESTREAM_H_ */
