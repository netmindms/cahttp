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
#include "testutil.h"

using namespace cahttp;
using namespace edft;
using namespace std;


TEST(req2, basic) {
	EdTask task;
	HttpReq req, req400;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.request_get("http://localhost:3000/simple", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					string data = req.fetchData();
					ASSERT_EQ(data.size(), 0);
					ASSERT_EQ(req.getRespStatus(), 200);
				}
			});
			req400.request_get("http://localhost:3000/status/400", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_END) {
					ali("request end,...");
					string data = req400.fetchData();
					ASSERT_EQ(data.size(), 0);
					ASSERT_EQ(req400.getRespStatus(), 400);
				}
			});
			task.setTimer(1, 500);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			req400.close();
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
}

TEST(req2, echo) {
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


TEST(req2, file) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
//			req.setReqContentFile("/home/netmind/.bashrc", "application/octet-stream");
			req.setReqContentFile(get_test_file_path(), "application/octet-stream");
			req.request_post("http://localhost:3000/upload", [&](HttpReq::Event event) {
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




TEST(req2, transfer_enc) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setReqContent("message should be echoed.", "application/octet-stream");
			req.transferEncoding(true);
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
			req.endData();
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
}

TEST(req2, transfer_enc_file) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setReqContentFile(get_test_file_path(), "application/octet-stream");
			req.transferEncoding(true);
			req.request_post("http://localhost:3000/upload", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ASSERT_EQ(req.getRespStatus(), 200);
					task.postExit();
				}
			});
			req.endData();
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
}
