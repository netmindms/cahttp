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
using namespace std;
void serverlive();


int main(int argc, char* argv[]) {
 	edft::EdNioInit();
//	serverlive();
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
//	::testing::GTEST_FLAG(filter) = "req2.*";
//	::testing::GTEST_FLAG(filter) = "req2.transfer_enc";
	::testing::GTEST_FLAG(filter) = "req2.transfer_enc_file";


	auto ret = RUN_ALL_TESTS();
	printf("test exit, ...\n");
	return ret;
}
