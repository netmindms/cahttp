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

namespace cahttpu {

#define FLB_FILE 1
#define FLB_DIR 2
#define FLB_EXT_CURDIR 3
#define FLB_EXT_UPDIR 4

class FileUtil
{
public:
	enum {
		FILE_ONLY=(1<<FLB_FILE),
		DIR_ONLY=(1<<FLB_DIR),
		DIR_FILE=(1<<FLB_FILE)|(1<<FLB_DIR),
	};
	FileUtil();
	virtual ~FileUtil();
	static bool isExist(const std::string& path);
	static std::vector<std::string> getFileList(const char* dirpath, uint32_t flag);
	static long getSize(const std::string &path);
};

} // namespace
#endif /* UTIL_FILEUTIL_H_ */
