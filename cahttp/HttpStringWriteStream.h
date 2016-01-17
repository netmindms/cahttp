/*
 * HttpStringWriteStream.h
 *
 *  Created on: Jul 24, 2015
 *      Author: netmind
 */

#ifndef SRC_HTTPSTRINGWRITESTREAM_H_
#define SRC_HTTPSTRINGWRITESTREAM_H_
#include <string>

#include "HttpBaseWriteStream.h"
namespace cahttp {
class HttpStringWriteStream : public HttpBaseWriteStream{
public:
	HttpStringWriteStream();
	virtual ~HttpStringWriteStream();
	ssize_t write(const char* buf, size_t len) override;
	size_t size() override;
	void end() override {};
	std::string fetchString();
	const std::string& getString();
private:
	std::string mStr;
};
}
#endif /* SRC_HTTPSTRINGWRITESTREAM_H_ */
