/*
 * HttpDataStream.h
 *
 *  Created on: Jul 21, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPBASEREADSTREAM_H_
#define SRC_HTTPBASEREADSTREAM_H_
#include <memory>
#include <string>
#include <utility>
namespace cahttp {
class HttpBaseReadStream {
public:
	HttpBaseReadStream();
	virtual ~HttpBaseReadStream();
//	virtual std::string readStr(size_t len)=0;
	virtual std::pair<const char*, int64_t> getDataPtr()=0;
	virtual size_t remain()=0;
	virtual void consume(size_t len)=0;
	virtual ssize_t store(const char* ptr, size_t len)=0;
};

typedef std::unique_ptr<HttpBaseReadStream> upHttpBaseReadStream;
}
#endif /* SRC_HTTPBASEREADSTREAM_H_ */
