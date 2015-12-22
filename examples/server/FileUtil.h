/*
 * FileUtil.h
 *
 *  Created on: Apr 4, 2015
 *      Author: netmind
 */

#ifndef UTIL_FILEUTIL_H_
#define UTIL_FILEUTIL_H_

#include <string>
#include <vector>
#include "etcutil.h"


#define FLB_FILE 1
#define FLB_DIR 2
#define FLB_EXT_CURDIR 4
#define FLB_EXT_UPDIR 8
#define FLB_SORT 16

class FileUtil
{
public:
	enum {
		FILE_ONLY=FLB_FILE,
		DIR_ONLY=FLB_DIR,
		DIR_FILE=FLB_FILE | FLB_DIR,
	};
	FileUtil();
	virtual ~FileUtil();
	static bool isExist(const std::string& path);
	static std::vector<std::string> getFileList(const char* dirpath, uint32_t flag);
};

#endif /* UTIL_FILEUTIL_H_ */
