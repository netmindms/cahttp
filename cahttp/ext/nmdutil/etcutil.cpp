/*
 * etcutil.cpp
 *
 *  Created on: Dec 3, 2015
 *      Author: netmind
 */


#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

namespace cahttpu {

int get_num_fds()
{
	int fd_count;
	char buf[300];
	struct dirent *dp;

	snprintf(buf, 256, "/proc/%i/fd/", getpid());

	fd_count = 0;
	DIR *dir = opendir(buf);
	while ((dp = readdir(dir)) != NULL)
	{
		//if(!(dp->d_type & DT_DIR))	logs("file = %s", dp->d_name);
		fd_count++;
	}
	closedir(dir);
	return fd_count;
}


}
