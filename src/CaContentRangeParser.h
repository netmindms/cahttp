/*
 * CaContentRangeParser.h
 *
 *  Created on: Oct 23, 2015
 *      Author: netmind
 */

#ifndef SRC_CACONTENTRANGEPARSER_H_
#define SRC_CACONTENTRANGEPARSER_H_
#include <sys/types.h>

namespace cahttp {
class CaContentRangeParser {
public:
	CaContentRangeParser();
	virtual ~CaContentRangeParser();
	int parse(const char* str, size_t len);
	void getOffset(size_t &start, size_t &end, size_t &total);
private:
	size_t mStart;
	size_t mEnd;
	size_t mTotalSize;
};
}
#endif /* SRC_CACONTENTRANGEPARSER_H_ */
