/*
 * test_svr2.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */



#define LOG_LEVEL LOG_DEBUG

#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/ReHttpServer.h".h"
#include "../cahttp/flog.h"
#include "../cahttp/CaHttpCommon.h"
#include "../cahttp/ReUrlCtrl.h"
#include "testutil.h"

using namespace cahttp;
using namespace edft;
using namespace std;


TEST(svr2, svr) {
	ReHttpServer::test();
//	class uctrl: public ReUrlCtrl {
//
//	};


//	auto *pctrl = svr.allocUrlCtrl(HTTP_GET, "/abc");
//	ASSERT_NE(pctrl, nullptr);
//	delete pctrl;
}

