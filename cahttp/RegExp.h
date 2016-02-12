/*
 * RegExp.h
 *
 *  Created on: Feb 12, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REGEXP_H_
#define CAHTTP_REGEXP_H_

#include <sys/types.h>
#include <regex.h>
#include <string>
#include <vector>
#include <utility>
namespace cahttp {

class RegExp {
public:
	RegExp();
	virtual ~RegExp();
	int setPattern(const char* pattern);
	std::pair<int, std::vector<std::string> > matchParams(const char* str);
	std::pair<int, std::vector<std::string> > matchParams(const std::string& str);
	void clear();
private:
	regex_t mRegex;
	regex_t *mpRegex;
};

} /* namespace cahttp */

#endif /* CAHTTP_REGEXP_H_ */
