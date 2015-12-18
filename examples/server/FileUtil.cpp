/*
 * FileUtil.cpp
 *
 *  Created on: Apr 4, 2015
 *      Author: netmind
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "FileUtil.h"

using namespace std;


FileUtil::FileUtil() {

}

FileUtil::~FileUtil() {
}

bool FileUtil::isExist(const std::string& path) {
	struct stat buf;
	auto ret = stat(path.data(), &buf);
	if (!ret)
		return true;
	else
		return false;
}

std::vector<std::string> FileUtil::getFileList(const char* dirpath, uint32_t flag) {
	DIR *dir;
	struct dirent *ent;
	vector<string> result;
	if ((dir = opendir(dirpath)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if(ent->d_type == DT_DIR) {
				if(BIT_TEST(flag, FLB_DIR)) {
					result.emplace_back(ent->d_name);
				}
			} else if(ent->d_type == DT_REG || ent->d_type == DT_LNK) {
				if(BIT_TEST(flag, FLB_FILE)) {
					result.emplace_back(ent->d_name);
				}
			}
		}
		closedir(dir);
	}
	return move(result);
}

