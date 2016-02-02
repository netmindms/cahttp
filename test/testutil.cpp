/*
 * testutil.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */


#include <sys/utsname.h>
#include <string>

using namespace std;

string get_test_file_path() {
	struct utsname un;
	uname(&un);
	string path = string("/boot/initrd.img-") + un.release;
	return path;
}
