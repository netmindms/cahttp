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
	int status_code=0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setReqContentFile("/home/netmind/temp/body.data", "application/octet-stream");
			req.transferEncoding(true);
			req.request_post("http://localhost:3000/upload", [&](HttpReq::Event event) {
//			req.request_post("http://192.168.5.12:3000/upload", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					status_code = req.getRespStatus();
					ali("req end, satus_code=%d", status_code);
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
	ASSERT_EQ(status_code, 200);
}


TEST(req2, manualdata) {
	EdTask task;
	HttpReq req;
	FILE* mSt;
	char fbuf[4*1024];
	size_t dataSize;
	size_t readFileCnt=0;

	int status_code=0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			dataSize = 0;
			mSt = fopen(get_test_file_path().data(), "rb");
			assert(mSt);
			req.addReqHdr(CAS::HS_CONTENT_TYPE, CAS::CT_APP_OCTET);
			req.transferEncoding(true);
			req.request_post("http://localhost:3000/upload", [&](HttpReq::Event event) {
				if(event == HttpReq::ON_MSG) {
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_SEND) {
					ald("on send, ");
					for(;;) {
						if(dataSize==0) {
							auto rcnt = fread(fbuf, 1, 4*1024, mSt);
							if(rcnt>0) {
								readFileCnt += rcnt;
								dataSize = rcnt;
							} else {
								ali("*** no file read data");
							}
						}
						if(dataSize>0) {
							if(dataSize != 4096) {
								ali("*** last data=%ld", dataSize);
							}
							auto sret = req.sendData(fbuf, dataSize);
							if(sret<=0) {
								dataSize = 0;
								if(sret<0) break;
							} else {
								ale("send paused...");
								break;
							}
						} else {
							req.endData();
							break;
						}
					}
				} else if(event == HttpReq::ON_END) {
					status_code = req.getRespStatus();
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			fclose(mSt);
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ali("fread cnt=%ld", readFileCnt);
	ASSERT_EQ(status_code, 200);
}

