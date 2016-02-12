/*
 * test_etc.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: netmind
 */



#define LOG_LEVEL LOG_INFO

#include <gtest/gtest.h>

#include "../cahttp/RegExp.h"

//using namespace edft;
using namespace cahttp;
using namespace std;

TEST(etc, reg) {
	RegExp reg;
	const char *pa = R"(/([0-9]+)/([0-9]+))";
	auto r = reg.setPattern(pa);
	ASSERT_EQ(r, 0);
	auto res = reg.matchParams("/123/456");
	ASSERT_EQ(res.first, 0);
	auto &vs = res.second;
	ASSERT_STREQ("123", vs[0].data());
	ASSERT_STREQ("456", vs[1].data());

	string ts = "/00/2323";
	res = reg.matchParams(ts);
	ASSERT_EQ(res.first, 0);
	auto &vs2 = res.second;
	ASSERT_STREQ("00", vs2[0].data());
	ASSERT_STREQ("2323", vs2[1].data());


	reg.clear();
	reg.setPattern(R"(/abc/123)");
	res = reg.matchParams("/123/23");
	ASSERT_NE(res.first, 0);
}
