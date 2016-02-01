/*
 * test_req2.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/HttpReq.h"
#include "../cahttp/flog.h"

using namespace cahttp;
using namespace edft;
using namespace std;

TEST(req2, basic) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setReqContent("message should be echoed.", "application/octet-stream");
			req.request_post("http://localhost:3000/echo", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					auto data = req.fetchData();
					ali("  recv data=%s", data);
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
}


