/*
 * test_svr2.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */



#define LOG_LEVEL LOG_DEBUG

#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/ReHttpServer.h"
#include "../cahttp/flog.h"
#include "../cahttp/CaHttpCommon.h"
#include "../cahttp/ReUrlCtrl.h"
#include "testutil.h"

using namespace cahttp;
using namespace edft;
using namespace std;


TEST(svr2, svr) {
//	ReHttpServer::test();
	class TestUrl: public ReUrlCtrl {
		string strBuf;
		virtual void OnHttpReqMsg(BaseMsg& msg) override {
			ali("on req message");
			response_file(200, "/home/netmind/temp/h");
//			response(200, "hello", CAS::CT_TEXT_PLAIN);
//			response(200, nullptr, 10, CAS::CT_TEXT_PLAIN.data());
		};
		virtual void OnHttpReqData(std::string&& data) override {
			ali("on req data");
			strBuf += data;
		};
		virtual void OnHttpEnd() override {
			ali("on req end");
			ali("  recv data=%s", strBuf);
		};
	};
	EdTask mTask;
	ReHttpServer mSvr;
	EdTimer mTimer;
	mTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mSvr.setUrlReg<TestUrl>(HTTP_GET, "/test");
			mSvr.setUrlReg<TestUrl>(HTTP_POST, "/test");
			mSvr.start(0);
		} else if(msg.msgid == EDM_CLOSE) {
			mTimer.kill();
			mSvr.close();
		}
		return 0;
	});
	mTask.runMain();
}

