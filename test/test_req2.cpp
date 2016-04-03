/*
 * test_req2.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#define LOG_LEVEL LOG_DEBUG

#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ednio/EdNio.h>
#include "../cahttp/HttpReq.h"
#include "../cahttp/flog.h"
#include "../cahttp/CaHttpCommon.h"
//#include "../cahttp/ReqMan.h"
#include "../cahttp/AsyncFile.h"
#include "../cahttp/HttpCnnMan.h"

#include "testutil.h"
#include "test_common.h"

#define TEMP_FILE_PATH "/tmp/upload.dat"
using namespace cahttp;
using namespace edft;
using namespace std;


static int check_result_file(const string &path=TEMP_FILE_PATH) {
	auto cmd = "diff " + get_test_file_path() + " " + path;
	auto diffret = system(cmd.data());
	return diffret;
}


class Req2Test :public ::testing::Test {
public:
	FILE *mSt;
	int mStartFdNum;
	void SetUp() override {
		ali("setup");
		FDCHK_SV(mStartFdNum);
		char dir[1024];
		getcwd(dir, 1024);
		string cmd = string("nodejs ")+dir+"/test/refserver.js";
		mSt = popen(cmd.c_str(), "r");
		if(mSt) {
			char buf[1024];
			auto rcnt = fgets(buf, 1024, mSt);
			if(rcnt>0) {
				string s = buf;
				auto r = s.find("refserver start listening");
				if(r == s.npos) {
					assert(0);
				}
			}
		} else {
			assert(0);
		}

	}

	void TearDown() override {
		ali("tear down");
		system("curl localhost:7000/exit");
		usleep(100*100);
		fclose(mSt);
		FDCHK_SV(mStartFdNum);
	}
};



TEST_F(Req2Test, basic) {
	EdTask task;
	HttpReq req, req404;
	int r1=0, r2=0;
	int fdn1, fdn2;
	uint8_t result=0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			FDCHK_SV(fdn1);
			ali("task init");
			req.request_get("http://localhost:7000/hello", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					r1 = req.getRespStatus();
					result |= 1;
					ali("simple request end,..., status=%d", r1);
					if(result==0x03) {
						task.postExit();
					}
				}
			});
			req404.request_get("http://localhost:7000/status/404", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					r2 = req404.getRespStatus();
					result |= 2;
					ali("404 request end,... status=%d", req404.getRespStatus());
					if(result==0x03) {
						task.postExit();
					}
				}
			});

		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			req404.close();
			FDCHK_EV(fdn2);
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(r1, 200);
	ASSERT_EQ(r2, 404);
}

TEST_F(Req2Test, basic_force_close) {
	EdTask task;
	HttpReq req, req404;
	int r1=0, r2=0;
	int fdn1, fdn2;
	int err2=-1;
	uint8_t result=0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			FDCHK_SV(fdn1);
			ali("task init");
			req.request_get("http://localhost:7000/hello", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					assert(0);
				}
			});
			req404.request_get("http://10.10.10.10:7000/status/404", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					err2 = err;
				}
			});
			req.close();
			task.setTimer(1, 1000);

		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			req404.close();
			FDCHK_EV(fdn2);
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
}

TEST_F(Req2Test, echo) {
	EdTask task;
	HttpReq req;
	string data = "echo";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.request(HTTP_POST, "http://localhost:7000/echo", data, "application/octet-stream", [&](HttpReq::Event event, int err) {
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


TEST_F(Req2Test, echo_te) {
	EdTask task;
	HttpReq req;
	string data = "echo transfer encoding";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.transferEncoding(true);
			req.request(HTTP_POST, "http://localhost:7000/echo", data, "application/octet-stream", [&](HttpReq::Event event, int err) {
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

TEST_F(Req2Test, file) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
//			req.setContentFile(get_test_file_path().data(), "application/octet-stream");
			req.setContentInfoFile(get_test_file_path().data(), CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/upload", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					assert(err==0);
					assert(req.getRespStatus()==200);
					auto data = req.fetchData();
					ali("  recv data=%s", data);
					task.postExit();
				}
			});
			auto r = req.sendContentFile(get_test_file_path().c_str());
			assert(!r);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	usleep(1000*100); // waiting for server writing uploaded file.
	ASSERT_EQ(check_result_file(), 0);
}



TEST_F(Req2Test, file_te) {
	EdTask task;
	HttpReq req;
	remove_test_file();
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
//			req.setReqContentFile("/home/netmind/.bashrc", "application/octet-stream");
//			req.setContentInfoFile("/home/netmind/temp/a", "application/octet-stream");
//			req.setContentInfoFile(get_test_file_path().data(), "application/octet-stream");
			req.setContentInfoFile(get_test_file_path().data(), CAS::CT_APP_OCTET);
			req.transferEncoding(true);
//			req.setContentFile(get_test_file_path().data(), "application/octet-stream");
			req.request_post("http://localhost:7000/upload", [&](HttpReq::Event event, int err) {
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
			auto r = req.sendContentFile(get_test_file_path().c_str());
			assert(!r);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	usleep(1000*100); // waiting for server writing uploaded file.
	ASSERT_EQ(check_result_file(), 0);
}



TEST_F(Req2Test, transfer_enc) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.addHeader(CAS::HS_CONTENT_TYPE, "application/octet-stream");
			req.transferEncoding(true);
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
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


TEST_F(Req2Test, echo_manual) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string data = "echo manual";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setContentInfo(data.size(), CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_SEND) {
					ali("on data");
					auto rr = req.sendData(data.c_str(), data.size());
					assert(rr==SR::eOk);
				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
					task.postExit();
				}
			});
			auto r = req.sendData(data.c_str(), data.size());
			assert(r == cahttp::SR::eNext);
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

TEST_F(Req2Test, echo_manual_tec_zero) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.transferEncoding(true);
			req.setContentType(CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_SEND) {

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


TEST_F(Req2Test, file_manual) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string recvdata;
	FILE* stFile;
	size_t bufSize=100*1024;
	unique_ptr<char[]> readBuf(new char[bufSize]);
	size_t readCnt=0;
	remove_test_file();
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			stFile = fopen(get_test_file_path().c_str(), "rb");
			assert(stFile);
			req.setContentInfoFile(get_test_file_path().c_str(), CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/upload", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_SEND) {

				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
					task.postExit();
				}
			});
			task.setTimer(1, 1);
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			fclose(stFile);
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			for(;;) {
				if(readCnt == 0) {
					readCnt = fread(readBuf.get(), 1, bufSize, stFile);
	//				ald("read cnt =%ld", readCnt);
					if(!readCnt) {
						ali("file read end...");
						task.killTimer(1);
					}
				}
				if(readCnt>0) {
					auto r = req.sendData(readBuf.get(), readCnt);
					if(r == SR::eOk || r == SR::ePending) {
						readCnt = 0;
					} else if(r == SR::eNext) {
						ald("postpone sending data");
						break;
					}
				} else {
					break;
				}
			}
		}
		return 0;
	});
	task.runMain();
	usleep(1000*100); // waiting for server writing uploaded file.
	ASSERT_EQ(check_result_file(), 0);
	FDCHK_E(1);
}

TEST_F(Req2Test, file_manual_on_send_seq) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string recvdata;
	FILE* stFile;
	size_t bufSize=100*1024;
	unique_ptr<char[]> readBuf(new char[bufSize]);
	size_t readCnt=0;
	remove_test_file();
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			stFile = fopen(get_test_file_path().c_str(), "rb");
			assert(stFile);
			req.setContentInfoFile(get_test_file_path().c_str(), CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/upload", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_SEND) {
					for(;;) {
						if(readCnt == 0) {
							readCnt = fread(readBuf.get(), 1, bufSize, stFile);
							//				ald("read cnt =%ld", readCnt);
							if(!readCnt) {
								ali("file read end...");
								task.killTimer(1);
							}
						}
						if(readCnt>0) {
							auto r = req.sendData(readBuf.get(), readCnt);
							if(r == SR::eOk || r == SR::ePending) {
								readCnt = 0;
							} else if(r == SR::eNext) {
								ald("postpone sending data");
								break;
							}
						} else {
							break;
						}
					}
				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			fclose(stFile);
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	usleep(1000*100); // waiting for server writing uploaded file.
	ASSERT_EQ(check_result_file(), 0);
	FDCHK_E(1);
}



TEST_F(Req2Test, file_manual_reserve_write) {
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string recvdata;
	FILE* stFile;
	size_t bufSize=2*1024;
	unique_ptr<char[]> readBuf(new char[bufSize]);
	size_t readCnt=0;
	remove_test_file();
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			stFile = fopen(get_test_file_path().c_str(), "rb");
			assert(stFile);
			req.setContentInfoFile(get_test_file_path().c_str(), CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/upload", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_SEND) {
					if(readCnt == 0) {
						readCnt = fread(readBuf.get(), 1, bufSize, stFile);
						if(!readCnt) {
							ali("file read end...");
						}
					}
					if(readCnt>0) {
						auto r = req.sendData(readBuf.get(), readCnt);
						if(r == SR::eOk || r == SR::ePending) {
							readCnt = 0;
						} else if(r == SR::eNext) {
							ald("postpone sending data");
						}
					}
					req.reserveWrite();
				} else if(event == HttpReq::ON_END) {
					ali("request end,...");
					recvdata = req.fetchData();
					ali("  recv data=%s", recvdata);
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			fclose(stFile);
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	usleep(1000*100); // waiting for server writing uploaded file.
	ASSERT_EQ(check_result_file(), 0);
	FDCHK_E(1);
}


TEST_F(Req2Test, echo_manual_tec) {
	EdTask task;
	HttpReq req;
	string data = "echo manual tec";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.transferEncoding(true);
			req.setContentType(CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_SEND) {
					auto r = req.sendData(data.data(), data.size());
					assert(r==SR::eOk);
					req.endData();
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


TEST_F(Req2Test, reuse) {
//	auto ss = sizeof(BaseMsg::status_t);
	FDCHK_S(1);
	EdTask task;
	HttpReq req;
	string data1 = "echo\n";
	string data2 = "echo reused 2\n";
	string data3 = "echo reused 3\n";
	string recv1, recv2, recv3;
	int chkfd;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			chkfd = cahttpu::get_num_fds();
			req.request(HTTP_POST, "http://localhost:7000/echo", data1, "application/octet-stream", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					ali("request end,...");
					recv1 = req.fetchData();
					ali("  recv data=%s", recv1);
					task.postMsg(EDM_USER);
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			task.killTimer(1);
			auto tfd = cahttpu::get_num_fds();
			assert(tfd == chkfd);
			ali("task closed");
		} else if(msg.msgid == EDM_USER) {
			req.clear();
			ali("start second request...");
			req.transferEncoding(true);
			req.setContentType(CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					ali("second req end, ");
					recv2 = req.fetchData();
					task.postMsg(EDM_USER+1);
				}
			});
			req.sendContent(data2.data(), data2.size());
		} else if(msg.msgid == EDM_USER+1) {
			req.clear();
			ali("start third request...");
			req.setContentInfo(data3.size(), CAS::CT_APP_OCTET);
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_END) {
					ali("third req end, ");
					recv3 = req.fetchData();
					task.postExit();
				}
			});
			req.sendContent(data3.data(), data3.size());
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
	ASSERT_STREQ(recv1.c_str(), data1.c_str());
	ASSERT_STREQ(recv2.c_str(), data2.c_str());
	ASSERT_STREQ(recv3.c_str(), data3.c_str());
	FDCHK_E(1);
}


#if 0
TEST_F(Req2Test, reqman) {
	EdTask task;
	ReqMan man;
	HttpReq *preq;
	string recvData;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			preq = man.getReq(inet_addr("127.0.0.1"), 7000);
			preq->request(HTTP_POST, "http://localhost:7000/echo", "echo", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					ali("on end");
					recvData = preq->fetchData();
					preq->close();
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			man.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_STREQ(recvData.c_str(), "echo");
}



TEST_F(Req2Test, reqman_chain) {
	EdTask task;
	ReqMan man;
	HttpReq *preq, *preq2;
	string recvData, recvData2;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			preq = man.getReq(inet_addr("127.0.0.1"), 7000);
			preq->request(HTTP_POST, "http://localhost:7000/echo", "echo", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					ali("on end");
					recvData = preq->fetchData();
					preq->close();
					preq2 = man.getReq(inet_addr("127.0.0.1"), 7000);
					preq2->request(HTTP_POST, "http://localhost:7000/echo", "echo2", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
						if(evt == HttpReq::ON_END) {
							ali("on end");
							recvData2 = preq2->fetchData();
							preq2->close();

						}
					});
					auto r = preq->getError();
					ali("req1 err=%d", r);
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			man.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_STREQ(recvData.c_str(), "echo");
	ASSERT_STREQ(recvData2.c_str(), "echo2");
}

TEST_F(Req2Test, reqman_premature_close) {
	EdTask task;
	ReqMan man;
	HttpReq *preq;
	int fdn;

	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			FDCHK_SV(fdn);
			preq = man.getReq(inet_addr("10.0.0.1"), 7000);
			preq->request(HTTP_POST, "http://10.0.0.1:7000/echo", "echo", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					ali("on end");
					preq->close();
				}
			});
			task.postExit();
		} else if(msg.msgid == EDM_CLOSE) {
			ali("task closed");
			man.close();
			FDCHK_EV(fdn);
		}
		return 0;
	});
	task.runMain();
}


TEST_F(Req2Test, reqman_pipe) {
	EdTask task;
	ReqMan man;
	HttpReq *preq1, *preq2, *preq3;
	string recvData;
	int resp1, resp2, resp3;
	string rdata1, rdata2, rdata3;

	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			preq1 = man.getReq(inet_addr("127.0.0.1"), 7000);
			preq1->request_get("http://localhost:7000/delay", [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					resp1 = preq1->getRespStatus();
					rdata1 = preq1->fetchData();
					ali("req1 end, status=%d", resp1);
					ASSERT_EQ(resp1, 200);

				}
			});

			preq2 = man.getReq(inet_addr("127.0.0.1"), 7000);
			preq2->request(HTTP_POST, "http://localhost:7000/echo", "echo\n", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					ali("req2 end, status=%d", preq2->getRespStatus());
					resp2 = preq2->getRespStatus();
					rdata2 = preq2->fetchData();
					ASSERT_EQ(resp2, 200);
					recvData = preq2->fetchData();
				}
			});

			preq3 = man.getReq(inet_addr("127.0.0.1"), 7000);
			preq3->transferEncoding(true);
			preq3->setContentType(CAS::CT_APP_OCTET);
			preq3->request_post("http://localhost:7000/upload", [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					ali("req3 end, status=%d", preq3->getRespStatus());
					resp3 = preq3->getRespStatus();
					ASSERT_EQ(resp3, 200);
					task.postExit();
				}
			});
			preq3->sendContentFile(get_test_file_path().c_str());
		} else if(msg.msgid == EDM_CLOSE) {
			man.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(resp1, 200);
	ASSERT_STREQ(rdata1.c_str(), "delayed response\n");
	ASSERT_EQ(resp2, 200);
	ASSERT_STREQ(rdata2.c_str(), "echo\n");
	ASSERT_EQ(resp3, 200);
	ASSERT_EQ(check_result_file(), 0);
}

TEST_F(Req2Test, reqman_idle_timer) {
	EdTask task;
	ReqMan man;
	HttpReq *preq;
	int fdn;
	size_t dummyCnt=0;

	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			FDCHK_SV(fdn);
			preq = man.getReq(inet_addr("127.0.0.1"), 7000);
			preq->request(HTTP_POST, "http://127.0.0.1:7000/echo", "echo", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					ali("on end");
					task.setTimer(1, 7000);
					preq->close();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			ali("task closed");
			man.close();
			FDCHK_EV(fdn);
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			dummyCnt = man.dbgGetCnnDummyPoolSize();
			task.postExit();
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(dummyCnt, 1);
}
#endif

TEST_F(Req2Test, send_data) {
	EdTask task;
	HttpReq req;
	string data = "echo send_data";
	string recvdata;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setContentType(CAS::CT_APP_OCTET);
			req.setContentLen(data.size());
			req.request_post("http://localhost:7000/echo", [&](HttpReq::Event event, int err) {
				if(event == HttpReq::ON_MSG) {
					ali("resonsed, status=%d, content_len=%ld", req.getRespStatus(), req.getRespContentLen());
				} else if(event == HttpReq::ON_DATA) {

				} else if(event == HttpReq::ON_SEND) {
					ali("on send event");
					string s1="echo ";
					string s2="send_data";
					auto sret = req.sendData(s1.data(), s1.size());
					assert(sret==SR::eOk);
					sret = req.sendData(s2.data(), s2.size());
					assert(sret==SR::eOk);
					sret = req.sendData(data.data(), data.size());
					assert(sret == SR::eFail);

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


TEST_F(Req2Test, forceclose) {
	EdTask task;
	HttpReq req;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.request_get("http://10.10.10.10", [&](HttpReq::Event evt, int err) {

			});
			task.setTimer(1, 1000);
		} else if(msg.msgid == EDM_CLOSE) {
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			req.close();
			task.postExit();
		}
		return 0;
	});
	task.runMain();
}

TEST_F(Req2Test, cnnman) {
	EdTask task;
	HttpReq req;
	HttpCnnMan cnnMan;
	string respData;
	int respStatus=0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			req.setCnnMan(cnnMan);
			req.request(HTTP_POST, "http://localhost:7000/echo", "echo", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					respStatus = req.getRespStatus();
					auto chcnt = cnnMan.getChannelCount(0);
					assert(chcnt.first==0);
					assert(chcnt.second==0);
					task.postExit();
				} else if(evt == HttpReq::ON_DATA) {
					respData.append( req.fetchData() );
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			req.close();
			cnnMan.close();
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			if(cnnMan.getPoolSize()==0) {
				task.killTimer(1);
				task.postExit();
			}
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(respStatus, 200);
	ASSERT_STREQ(respData.c_str(), "echo");
}




TEST_F(Req2Test, cnnman_pipe) {
	EdTask task;
	HttpReq req1, req2;
	string rs2Data;
	HttpCnnMan cnnMan;
	int rs1=-1, rs2=-1;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			cnnMan.pipelining(true);
			req1.setCnnMan(cnnMan);
			req2.setCnnMan(cnnMan);

			req1.request_get("http://localhost:7000/delay", [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					assert(rs2==-1);
					rs1 = req1.getRespStatus();
				}
			});
			req2.request(HTTP_POST, "http://localhost:7000/echo", "echo2", CAS::CT_APP_OCTET, [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					assert(rs1 != -1);
					rs2 = req2.getRespStatus();
					rs2Data = req2.fetchData();
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			req1.close();
			req2.close();
			cnnMan.close();
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			if(cnnMan.getPoolSize()==0) {
				task.killTimer(1);
				task.postExit();
			}
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(rs1, 200);
	ASSERT_EQ(rs2, 200);
	ASSERT_STREQ(rs2Data.c_str(), "echo2");
}

#if 0

TEST_F(Req2Test, cnnman_force_close) {
	EdTask task;
	vector<unique_ptr<HttpReq>> reqs;
	HttpCnnMan cnnMan;
	int respStatus=0;
	int respCnt = 0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			for(int i=0;i<10;i++) {
				reqs.emplace_back(new HttpReq);
				auto &arq = (*reqs.back());
				reqs[i]->setCnnMan(cnnMan);
				string s = "echo-"+to_string(i);
				reqs[i]->request(HTTP_POST, "http://10.10.10.10:7000/echo", s, CAS::CT_APP_JSON, [&](HttpReq::Event evt, int err) {
					if(evt == HttpReq::ON_END) {
						assert(arq.getRespStatus()==0);
						assert(err == ERR::eEarlyDisconnected );
						++respCnt;
					}
				});
			}
			reqs[0]->close();
			task.setTimer(1, 500);
		} else if(msg.msgid == EDM_CLOSE) {
			for(auto &r: reqs) {
				r->close();
			}
			cnnMan.close();
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
}



TEST_F(Req2Test, cnnman_pipe_force_close) {
	EdTask task;
	vector<unique_ptr<HttpReq>> reqs;
	HttpCnnMan cnnMan;
	int respStatus=0;
	int respCnt = 0;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			cnnMan.pipelining(true);
			for(int i=0;i<10;i++) {
				reqs.emplace_back(new HttpReq);
				auto &arq = (*reqs.back());
				reqs[i]->setCnnMan(cnnMan);
				string s = "echo-"+to_string(i);
				reqs[i]->request(HTTP_POST, "http://10.10.10.10:7000/echo", s, CAS::CT_APP_JSON, [&](HttpReq::Event evt, int err) {
					if(evt == HttpReq::ON_END) {
						assert(arq.getRespStatus()==0);
						assert(err == ERR::eEarlyDisconnected );
						++respCnt;
					}
				});
			}
			reqs[0]->close();
			task.setTimer(1, 500);
		} else if(msg.msgid == EDM_CLOSE) {
			for(auto &r: reqs) {
				r->close();
			}
			cnnMan.close();
			ali("task closed");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
}

TEST_F(Req2Test, timeout) {
	EdTask task;
	HttpReq req;
	size_t fdn;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			FDCHK_SV(fdn);
			req.request_get("http://10.10.10.10", [&](HttpReq::Event evt, int err) {
				if(evt == HttpReq::ON_END) {
					auto r = req.getRespStatus();
					ali("expected err=%s", cahttp_err_str(req.getError()));
					ASSERT_EQ(r, 0);
					ASSERT_EQ(err, ERR::eNoResponse);
					task.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			FDCHK_EV(fdn);
			req.close();
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
}

TEST_F(Req2Test, perf) {
	EdTask task;
	EdTimer mTimer;
	EdEventFd mEvtFd;
	vector<unique_ptr<HttpReq>> Reqs;
	size_t ReqCnt;
	size_t RespCnt;
	size_t totalCnt=1;
	task.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			ali("task init");
			ReqCnt = 0;
			RespCnt = 0;
			ali("start requesting flow...");
			mEvtFd.open([&](int cnt){
//				for(int i=0;i<10 && Reqs.size()<totalCnt; i++) {
					unique_ptr<HttpReq> preq ( new HttpReq);
					auto *myreq = preq.get();
					preq->request_get("http://localhost:7000/hello", [&mEvtFd, &totalCnt, ReqCnt, &RespCnt, myreq, &task](HttpReq::Event evt, int err) {
						if(evt == HttpReq::ON_END) {
							auto r = myreq->getRespStatus();
							assert(r==200);
							assert(err==0);
							RespCnt++;
							if(RespCnt >= totalCnt) {
								task.postExit();
							}
							myreq->close();
						}
					});
					Reqs.push_back(move(preq));
					if(Reqs.size()>=totalCnt) {
						ali("stop flow");
//						mTimer.kill();
						mEvtFd.close();
					} else {
						mEvtFd.raise();

					}
//				} // for loop
			});
			mEvtFd.raise();
//			req.request_get("http://localhost:7000", [&](HttpReq::Event evt, int err) {
//				if(evt == HttpReq::ON_END) {
//					auto r = req.getRespStatus();
//					ali("expected err=%s", cahttp_err_str(req.getError()));
//					ASSERT_EQ(r, 0);
//					ASSERT_EQ(err, ERR::eNoResponse);
//					task.postExit();
//				}
//			});
		} else if(msg.msgid == EDM_CLOSE) {
			ali("task closed");
		}
		return 0;
	});
	task.runMain();
	ASSERT_EQ(RespCnt, totalCnt);
	Reqs.clear();
}

#endif

