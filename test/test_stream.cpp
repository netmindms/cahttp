/*
 * test_stream.cpp
 *
 *  Created on: Jan 17, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include <gtest/gtest.h>
#include "../cahttp/MultiStream.h"
#include "../cahttp/HttpStringReadStream.h"
#include "../cahttp/flog.h"
#include "../cahttp/TransfEncStrm.h"
#include "../cahttp/ext/nmdutil/nmdu_format.h"
using namespace std;
using namespace cahttp;

TEST(strm, 0) {
	MultiStream mstrm;
	HttpStringReadStream s1;
	HttpStringReadStream s2;
	s1.store("12345", 5);
	s2.store("abcde", 5);

	mstrm.addStrm(&s1);
	mstrm.addStrm(&s2);

	auto dp = mstrm.getDataPtr();
	ASSERT_EQ(dp.second, 5);
	mstrm.consume(dp.second);
	dp = mstrm.getDataPtr();
	ASSERT_EQ(dp.second, 5);
	mstrm.consume(dp.second);
	dp = mstrm.getDataPtr();
	ASSERT_EQ(dp.second, -1);

	ali("test end");
}

TEST(strm, transf_enc) {
	TransfEncStrm testrm;
	HttpStringReadStream ss;
	string s = "123456";
	ss.store(s.data(), s.size());
	testrm.setStream(&ss);
	auto dp = testrm.getDataPtr();
	string expstr = nmdu::fmt::format("{:x}\r\n", s.size());
	expstr += (s+"\r\n");
	string enc(dp.first, dp.second);
	ASSERT_STREQ(enc.data(), expstr.data());
	testrm.consume(dp.second);

	dp = testrm.getDataPtr();
	enc.assign(dp.first, dp.second);
	expstr = "0\r\n\r\n";
	ASSERT_STREQ(enc.data(), expstr.data());
	testrm.consume(dp.second);

	dp = testrm.getDataPtr();
	ASSERT_EQ(dp.second, 0);

}

