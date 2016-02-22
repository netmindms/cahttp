/*
 * testmain.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: netmind
 */

#include <gtest/gtest.h>
#include <sys/utsname.h>
#include <list>
#include <ednio/EdNio.h>
#include <bitset>

#include "../cahttp/flog.h"
#include "../cahttp/RegExp.h"
#include "../cahttp/ext/nmdutil/etcutil.h"
using namespace std;
void serverlive();

namespace nu = cahttpu;
union tt {
	uint8_t val;
	struct {
		unsigned char s:1;
		unsigned char v:1;
		unsigned char d:1;
		unsigned char z:1;
		unsigned char :0;
	};
};


int main(int argc, char* argv[]) {

//	cahttp::RegExp reg;
//	const char *pa = R"(/([0-9]+)/([0-9]+))";
//	auto r = reg.setPattern(pa);
//	auto res = reg.matchParams("/123/456");
//	auto &vs = res.second;
//
//	reg.clear();
//	reg.setPattern(R"(/abc/123)");
//	res = reg.matchParams("/123/23");
//	return 0;
 	edft::EdNioInit();
 	NMDU_SET_LOG_LEVEL(LOG_VERBOSE);
	::testing::InitGoogleTest(&argc, argv);
//	::testing::GTEST_FLAG(filter) = "strm.*";
//	::testing::GTEST_FLAG(filter) = "request.*";
//	::testing::GTEST_FLAG(filter) = "request.connection";
//	::testing::GTEST_FLAG(filter) = "request.recv_file";
//	::testing::GTEST_FLAG(filter) = "request.reqman";
//	::testing::GTEST_FLAG(filter) = "request.echo";
//	::testing::GTEST_FLAG(filter) = "request.stream_*";
//	::testing::GTEST_FLAG(filter) = "request.stream_send0*";
//	::testing::GTEST_FLAG(filter) = "request.stream_send_tec";
//	::testing::GTEST_FLAG(filter) = "request.manual_send";
//	::testing::GTEST_FLAG(filter) = "request.manual_send_tec";
//	::testing::GTEST_FLAG(filter) = "request.stream_send";
//	::testing::GTEST_FLAG(filter) = "request.simple";
	//	::testing::GTEST_FLAG(filter) = "ipc.mq";
//	::testing::GTEST_FLAG(filter) = "server.*";
//	::testing::GTEST_FLAG(filter) = "server.regex";
//	::testing::GTEST_FLAG(filter) = "server.tec";
//	::testing::GTEST_FLAG(filter) = "server.reqdata";
//	::testing::GTEST_FLAG(filter) = "server.manualsend";
//	::testing::GTEST_FLAG(filter) = "server.autosend";
//	::testing::GTEST_FLAG(filter) = "msg.tec";
//	::testing::GTEST_FLAG(filter) = "frame2.*";
//	::testing::GTEST_FLAG(filter) = "frame2.*";
//	::testing::GTEST_FLAG(filter) = "pktbuf.*";
	::testing::GTEST_FLAG(filter) = "msg.content";
//	::testing::GTEST_FLAG(filter) = "req2.reuse";
//	::testing::GTEST_FLAG(filter) = "req2.echo_manual_tec_zero";
//	::testing::GTEST_FLAG(filter) = "req2.transfer_enc";
//	::testing::GTEST_FLAG(filter) = "req2.transfer_enc_file";
//	::testing::GTEST_FLAG(filter) = "req2.manualdata";
//	::testing::GTEST_FLAG(filter) = "etc.*";
//	::testing::GTEST_FLAG(filter) = "svr2.*";


	auto ret = RUN_ALL_TESTS();
	printf("test exit, ...\n");
	return ret;
}
