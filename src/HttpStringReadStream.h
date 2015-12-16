/*
 * HttpStringReadStream.h
 *
 *  Created on: Jul 21, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPSTRINGREADSTREAM_H_
#define SRC_HTTPSTRINGREADSTREAM_H_
#include "HttpBaseReadStream.h"
namespace cahttp {
class HttpStringReadStream : public HttpBaseReadStream {
public:
	HttpStringReadStream();
	virtual ~HttpStringReadStream();
//	std::string readStr(size_t len) override;
	std::pair<const char*, int64_t> getDataPtr() override;
	void consume(size_t len) override;
	size_t remain() override;
	void setString(std::string &&s);
	const std::string& getString();
	ssize_t store(const char* ptr, size_t len) override;
private:
	std::string mStrBuf;
	size_t mReadPos;

};

typedef std::unique_ptr<HttpStringReadStream> upHttpStringReadStream;
}
#endif /* SRC_HTTPSTRINGREADSTREAM_H_ */
