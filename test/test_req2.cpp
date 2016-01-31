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
			req.request(HTTP_GET);
		} else if(msg.msgid == EDM_CLOSE) {

		}
		return 0;
	});
	task.runMain();
}
