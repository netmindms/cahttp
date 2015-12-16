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
	cout << "cahttp test main" << endl;
//	serverlive();
	::testing::InitGoogleTest(&argc, argv);
//	::testing::GTEST_FLAG(filter) = "request.*";
	::testing::GTEST_FLAG(filter) = "request.connection";
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
//	::testing::GTEST_FLAG(filter) = "server.tec";
//	::testing::GTEST_FLAG(filter) = "server.reqdata";
//	::testing::GTEST_FLAG(filter) = "server.manualsend";
//	::testing::GTEST_FLAG(filter) = "server.autosend";


	auto ret = RUN_ALL_TESTS();
	printf("test exit, ...\n");
	return ret;
}
