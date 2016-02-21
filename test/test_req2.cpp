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
#include "../cahttp/CaHttpCommon.h"
#include "testutil.h"
#include "test_common.h"

using namespace cahttp;
using namespace edft;
using namespace std;

static int check_result_file(const string &path="/tmp/upload.dat") {
	auto cmd = "diff " + get_test_file_path() + " " + path;
	auto diffret = system(cmd.data());
	return diffret;
}

TEST(req2, basic) {
	EdTask task;
	HttpReq req, req404;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.request_get("http://localhost:3000/simple", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
				}
			});
			req404.request_get("http://localhost:3000/status/404", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_END) {
					ali("request end,...");
				}
			});
			task.setTimer(1, 500);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			req404.close();
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(req.getRespStatus(), 200);
	ASSERT_EQ(req404.getRespStatus(), 404);
}

TEST(req2, echo) {
	EdTask task;
	HttpReq req;
	string data = "echo";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.request(HTTP_POST, "http://localhost:3000/echo", data, "application/octet-stream", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
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
	ASSERT_STREQ(recvdata.c_str(), data.c_str());
}

TEST(req2, echo_te) {
	EdTask task;
	HttpReq req;
	string data = "echo transfer encoding";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.transferEncoding(true);
			req.request(HTTP_POST, "http://localhost:3000/echo", data, "application/octet-stream", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
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
	ASSERT_STREQ(recvdata.c_str(), data.c_str());
}

TEST(req2, file) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setContentFile(get_test_file_path().data(), "application/octet-stream");
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
//			req.writeContentFile(get_test_file_path().c_str());
//			req.writeFile("/home/netmind/temp/a");
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(check_result_file(), 0);
}



TEST(req2, file_te) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
//			req.setReqContentFile("/home/netmind/.bashrc", "application/octet-stream");
//			req.setContentInfoFile("/home/netmind/temp/a", "application/octet-stream");
//			req.setContentInfoFile(get_test_file_path().data(), "application/octet-stream");
			req.transferEncoding(true);
			req.setContentFile(get_test_file_path().data(), "application/octet-stream");
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
//			req.writeContentFile(get_test_file_path().c_str());
//			req.writeFile("/home/netmind/temp/a");
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(check_result_file(), 0);
}



TEST(req2, transfer_enc) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.addHeader(CAS::HS_CONTENT_TYPE, "application/octet-stream");
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


TEST(req2, echo_manual) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string data = "echo manual";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setContentLen(data.size());
			req.setContentType(CAS::CT_APP_OCTET);
			req.request_post("http://localhost:3000/echo", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
					task.postExit();
				}
			});
			auto r = req.writeData(data.data(), data.size());
			assert(r==0);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_STREQ(recvdata.c_str(), data.c_str());
	FDCHK_E(1);
}

TEST(req2, echo_manual_tec_zero) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.transferEncoding(true);
			req.setContentType(CAS::CT_APP_OCTET);
			req.request_post("http://localhost:3000/echo", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
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
	ASSERT_EQ(recvdata.size(), 0);
	FDCHK_E(1);
}

TEST(req2, echo_manual_tec) {
	EdTask task;
	HttpReq req;
	string data = "echo manual tec";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.transferEncoding(true);
			req.setContentType(CAS::CT_APP_OCTET);
			req.request_post("http://localhost:3000/echo", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
					task.postExit();
				}
			});
			auto r = req.writeData(data.data(), data.size());
			req.endData();
			assert(r==0);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_STREQ(recvdata.c_str(), data.c_str());
}

TEST(req2, cnn_timeout) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.request_get("http://10.10.10.10", [&](HttpReq::Event evt) {
				if(evt == HttpReq::ON_END) {
					auto r = req.getRespStatus();
					ASSERT_EQ(r, 0);
					ali("expected err=%s", cahttp_err_str(req.getError()));
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
