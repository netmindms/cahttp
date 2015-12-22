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
#include <algorithm>
#include "FileUtil.h"

#include <string.h>
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
			bool badd=false;
			if(ent->d_type == DT_DIR) {
				if((flag & FLB_DIR)) {
					if(!strcmp(ent->d_name, "..") && !(flag & FLB_EXT_UPDIR)) {
						badd = true;
					} else if(strcmp(ent->d_name, ".")) {
						badd = true;
					}
				}
			} else if(ent->d_type == DT_REG) { // || ent->d_type == DT_LNK) {
				if(flag & FLB_FILE) {
					badd=true;
				}
			}

			if(badd) result.emplace_back(ent->d_name);
		}
		if(flag & FLB_SORT) {
			std::sort(result.begin(), result.end());
		}
		closedir(dir);
	}
	return move(result);
}

