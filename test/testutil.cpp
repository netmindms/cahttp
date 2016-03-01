/*
 * testutil.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */


#include <unistd.h>
#include <sys/utsname.h>
#include <string>

using namespace std;

string get_test_file_path() {
	struct utsname un;
	uname(&un);
	string path = string("/boot/initrd.img-") + un.release;
	return path;
}

void remove_test_file() {
	unlink( get_test_file_path().c_str());
}
