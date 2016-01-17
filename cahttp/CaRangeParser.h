/*
 * CaRangeHeader.h
 *
 *  Created on: Oct 23, 2015
 *      Author: netmind
 */

#ifndef SRC_CARANGEPARSER_H_
#define SRC_CARANGEPARSER_H_

#include <stdint.h>
#include <stdio.h>
#include <utility>
namespace cahttp {
class CaRangeParser {
public:
	CaRangeParser();
	virtual ~CaRangeParser();
	int parse(const char* str, size_t len);
	std::pair<long, long> getOffset(size_t total_size);
private:
	long mStart, mEnd;
	uint8_t mFlag;
};
}
#endif /* SRC_CARANGEPARSER_H_ */
