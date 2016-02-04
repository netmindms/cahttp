/*
 * test_buf.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */

#include <string>
#include <gtest/gtest.h>
#include <sys/utsname.h>

#include "../cahttp/FilePacketBuf.h"
#include "../cahttp/ext/nmdutil/FileUtil.h"

using namespace std;
using namespace cahttp;
using namespace cahttpu;

TEST(pktbuf, file) {
	FilePacketBuf fbuf;
	struct utsname un;
	uname(&un);
	string path = string("/boot/initrd.img-") + un.release;
	auto ret = fbuf.open(path);
	ASSERT_EQ(ret, 0);

	int64_t rcnt=0;
	auto filesize=FileUtil::getSize(path);
	for(;;) {
		auto bf = fbuf.getBuf();
		if(bf.first==0) {
			break;
		}
		rcnt += bf.first;
		fbuf.consume();
	}
	fbuf.close();
	ASSERT_EQ(filesize, rcnt);
}


