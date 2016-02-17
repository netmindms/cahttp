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
		virtual void OnHttpReqMsg(BaseMsg& msg) override {
			response(200, nullptr, 0, nullptr);
		};
		virtual void OnHttpReqData(std::string&& data) override {

		};
		virtual void OnHttpEnd() override {

		};
	};
	EdTask mTask;
	ReHttpServer mSvr;
	mTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mSvr.setUrlReg<TestUrl>(HTTP_GET, "/test");
			mSvr.start(0);
		} else if(msg.msgid == EDM_CLOSE) {
			mSvr.close();
		}
		return 0;
	});
	mTask.runMain();
}

