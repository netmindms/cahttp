#define LOG_LEVEL LOG_INFO

#include "../cahttp/flog.h"
#include <sys/utsname.h>
#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/HttpFileReadStream.h"
#include "../cahttp/HttpFileWriteStream.h"
#include "../cahttp/CaHttpServer.h"
#include "../cahttp/CaHttpUrlParser.h"
#include "../cahttp/CaHttpReqMan.h"

#define SERVER_PORT 9000

using namespace std;
using namespace edft;
using namespace cahttp;


static string TEST_FILE_NAME;
static string BASE_URL="http://127.0.0.1:9000";

class _INIT_TEST_REQUEST_MODULE {
public:
	_INIT_TEST_REQUEST_MODULE() {
		struct utsname un;
		uname(&un);
		TEST_FILE_NAME = string("/boot/initrd.img-") + un.release;
//		TEST_FILE_NAME = string("/home/netmind/temp/aa.dat");
	}
};
static _INIT_TEST_REQUEST_MODULE _this_module;

static string PYTHON_SCRIPT = ""
		"import BaseHTTPServer\n"
		"class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):\n"
		"  def do_GET(s):\n"
		"    s.protocol_version='HTTP/1.1'\n"
		"    s.send_response(200)\n"
		"    if s.path in ('/echo', '/index'):\n"
		"      s.send_header('Content-Type', 'application/octet-stream')\n"
		"      s.send_header('Content-Length', str(len('echo')))\n"
		"      s.end_headers()\n"
		"      s.wfile.write(s.path[1:])\n"
		"\n"
		"  def do_POST(s):\n"
		"    s.protocol_version='HTTP/1.1'\n"
		"    content_len = int(s.headers.getheader('content-length', 0))\n"
		"    data = s.rfile.read(content_len)\n"
		"    s.send_response(200)\n"
		"    if s.path == '/echo':\n"
		"      s.send_header('Content-Type', 'application/octet-stream')\n"
        "      s.send_header('Content-Length', str(len(data)))\n"
		"      s.end_headers()\n"
		"      s.wfile.write(data)\n"
		"    else if s.path == '/upload':\n"
		"      f = open('/tmp/upload.dat', 'wb')\n"
		"      f.write(data)\n"
		"      f.close()\n"
		"    if s.path == '/quit':\n"
		"      httpd.server_close()\n"
		"\n"
		"server_class = BaseHTTPServer.HTTPServer\n"
		"\n"
		"httpd = server_class(('127.0.0.1', SERVER_PORT), MyHandler)\n"
		"try:\n"
		"  httpd.serve_forever()\n"
		"except:\n"
		"  pass\n"
		"httpd.server_close()\n"
		"\n";

static EdTask gCaHttpSvrTask;

void run_server() {
	EdFile file;
	file.openFile("/tmp/svr.py", EdFile::OPEN_RWTC);
	file.writeFile(PYTHON_SCRIPT.data(), PYTHON_SCRIPT.size());
	file.closeFile();
//	system(("printf \"" + PYTHON_SCRIPT + "\" | python").data());
	system("python /tmp/svr.py");
}

static int check_result_file(const string &path) {
	auto cmd = "diff " + TEST_FILE_NAME + " " + path;
	auto diffret = system(cmd.data());
	return diffret;
}

static void do_cahttp_server() {
	class FileUpload: public CaHttpUrlCtrl {
		HttpFileWriteStream wstrm;
		void OnHttpReqMsgHdr() override {
			ali("req msg hdr...");
			auto fd = wstrm.open("/tmp/upload.dat");
			assert(fd>=0);
			setReqDataStream(&wstrm);
		}
		void OnHttpReqMsg() override {
			ali("file upload req...");
			response(200, "/tmp/upload.dat", "application/octet-stream");
		}
	};
	class EchoUrl: public CaHttpUrlCtrl {
		void OnHttpReqMsg() override {
			ali("echo req msg,");
			auto &urlstr = getReqUrlStr();
			CaHttpUrlParser parser;
			parser.parse(urlstr);
			if(parser.query!="") {
				unordered_map<string, string> kvmap;
				parser.parseKeyValueMap(&kvmap, parser.query);
				if(kvmap["param"] != "") {
					response(200, kvmap["param"], "application/octet-stream");
				}
			}
			response(403);
		}

		void OnHttpEnd() override {
			ali("echo req end,...");
		}
	};

	class FileDownloadUrl: public CaHttpUrlCtrl {
		HttpFileReadStream filestrm;
		void OnHttpReqMsg() override {
			ald("file download request, ...");
//			HttpFileReadStream* pstrm = new HttpFileReadStream;
			auto fd = filestrm.open(TEST_FILE_NAME.data());
			assert(fd>0);
			setRespContent(&filestrm, EdFile::getSize(TEST_FILE_NAME.data()));
			response(200);
		}
		void OnHttpEnd() override {
			ald("file download end...");
		}
	};

	static CaHttpServer mSvr;
	gCaHttpSvrTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mSvr.config("port", to_string(SERVER_PORT).data());
			mSvr.setUrl<FileUpload>(HTTP_POST, "/upload");
			mSvr.setUrl<EchoUrl>(HTTP_GET, "/echo");
			mSvr.setUrl<FileDownloadUrl>(HTTP_GET, "/filedownload");
			mSvr.start(0);
		} else if(msg.msgid == EDM_CLOSE) {
			mSvr.close();
		}

		return 0;
	});
	gCaHttpSvrTask.run();
}

static void quit_cahttp_server() {
	gCaHttpSvrTask.terminate();
}


pair<int, string> do_request(const string &url, string ipstr="", int port=0) {
	string mRespData;
	EdTask mTask;
	CaHttpReq mReq;
	int mCode;
	mTask.setOnListener([&](EdMsg& msg) {
		if(msg.msgid == EDM_INIT) {
			if(ipstr.empty()==false) {
				mReq.setRemoteHostAddr(ipstr.data(), port);
			}
//			mReq.setOnListener([&](int event, CaHttpReq &r) {
//				if(event == r.HTTP_RESP_MSG) {
//					mCode = r.getRespCode();
//					mRespData = r.getRespData();
//					ali("result: %s", mRespData);
//					mTask.postExit();
//				}
//			});
			mReq.request("GET", url, [&](CaHttpReq &r, int event, int status) {
				if(event == r.HTTP_REQ_END) {
					mCode = r.getRespCode();
					mRespData = r.getRespData();
					mTask.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			mReq.close();
		}
		return 0;
	});
	mTask.runMain();
	return {mCode, mRespData};
}

static pair<int, string> do_stream_send_request(bool tec) {
	EdTask mCliTask;
	CaHttpReq Req;
	HttpFileReadStream fstrm;
	int mCode;
	string mRespData;
	mCliTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			Req.setOnListener([&](CaHttpReq& r, int event, int status) {
				if(event == r.HTTP_REQ_END) {
					mCode = r.getRespCode();
					mRespData = r.getRespData();
					mCliTask.postExit();
				}
			});
			Req.setUrl("/upload");
			Req.setRemoteHostAddr("127.0.0.1", SERVER_PORT);
			auto fd = fstrm.open(TEST_FILE_NAME.data(), 4096);
			assert(fd>-1);
			Req.setReqContent(&fstrm, "application/octet-stream");
			if(tec) Req.enableTransferChunked();
			Req.request("POST");
		} else if(msg.msgid == EDM_CLOSE) {
			Req.close();
		}
		return 0;
	});
	mCliTask.runMain();
	return {mCode, mRespData};
}

static pair<int, string> do_manual_send_request(bool tec) {
	EdTask mTask;
	CaHttpReq mReq;
	int mCode;
	string mRespData;
	FILE* mSt;
	char mBuf[4096];
	size_t mDataCnt=0;
	mTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mSt = fopen(TEST_FILE_NAME.data(), "rb");

			mReq.setOnListener([&](CaHttpReq& r, int event, int status) {
				if(event == r.HTTP_REQ_END) {
					mCode = r.getRespCode();
					mRespData = r.getRespData();
					mTask.postExit();
				} else if(event == r.HTTP_REQ_DATA_UNDERRUN) {
					ssize_t rcnt=0;
					if(mDataCnt==0) {
						rcnt = fread(mBuf, 1, sizeof(mBuf), mSt);
						if(rcnt>0) mDataCnt = rcnt;
					}
					auto wret = r.sendData(mBuf, rcnt);
					if(!wret) {
						mDataCnt = 0;
					} else {
						ali("send fail,...");
					}
				}
			});
			mReq.setUrl("/upload");
			mReq.setRemoteHostAddr("127.0.0.1", SERVER_PORT);
			mReq.setReqContent(EdFile::getSize(TEST_FILE_NAME.data()), CAS::CT_APP_OCTET );
			if(tec) mReq.enableTransferChunked();
			mReq.request("POST");
		} else if(msg.msgid == EDM_CLOSE) {
			if(mSt) fclose(mSt);
			mReq.close();
		}
		return 0;
	});
	mTask.runMain();
	return {mCode, mRespData};
}

TEST(request, manual_send) {
	do_cahttp_server();
	auto res = do_manual_send_request(false);
	auto diff_result = check_result_file(res.second);
	ASSERT_EQ(diff_result, 0);
	quit_cahttp_server();
}

TEST(request, manual_send_tec) {
	do_cahttp_server();
	auto res = do_manual_send_request(true);
	auto diff_result = check_result_file(res.second);
	ASSERT_EQ(diff_result, 0);
	quit_cahttp_server();
}


TEST(request, stream_send) {
	do_cahttp_server();
	auto res = do_stream_send_request(false);
	ASSERT_EQ(res.first, 200);
	auto cmd = "diff " + TEST_FILE_NAME + " " + res.second;
	auto diffret = system(cmd.data());
	ASSERT_EQ(diffret, 0);
	quit_cahttp_server();
}


TEST(request, stream_send_tec) {
	do_cahttp_server();
	auto res = do_stream_send_request(true);
	ASSERT_EQ(res.first, 200);
	auto cmd = "diff " + TEST_FILE_NAME + " " + res.second;
	auto diffret = system(cmd.data());
	ASSERT_EQ(diffret, 0);
	quit_cahttp_server();
}


TEST(request, echo) {
	do_cahttp_server();
	{
		auto res = do_request("/echo?param=hello", "127.0.0.1", SERVER_PORT);
		ASSERT_EQ(res.first, 200);
		ASSERT_STREQ(res.second.data(), "hello");
	}

	{
		auto res = do_request("http://127.0.0.1:"+to_string(SERVER_PORT)+"/echo?param=hello");
		ASSERT_EQ(res.first, 200);
		ASSERT_STREQ(res.second.data(), "hello");
	}

	quit_cahttp_server();
}

TEST(request, recv_file) {
	do_cahttp_server();
	EdTask mCliTask;
	CaHttpReq mReq;
	HttpFileWriteStream fwstrm;
	mCliTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			fwstrm.open("/tmp/download.dat");
			mReq.setRemoteHostAddr("127.0.0.1", SERVER_PORT);
			mReq.setRespDataStream(&fwstrm);
			mReq.request_get("/filedownload", [&](CaHttpReq &req, int event, int status) {
				if(event == req.HTTP_REQ_END) {
					mCliTask.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {
			mReq.close();
		}
		return 0;
	});
	mCliTask.runMain();
	auto file_diff = check_result_file("/tmp/download.dat");
	ASSERT_EQ(file_diff, 0);
	quit_cahttp_server();
}

TEST(request, reqman) {
	do_cahttp_server();

	EdTask mTask;
	CaHttpReqMan mMan;
	CaHttpReq *mpReq[10];
	string mRespData[10];
	int mCode[10];
	mTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mpReq[0] = mMan.newRequest();
			mpReq[0]->setRemoteHostAddr("127.0.0.1", SERVER_PORT);
			mpReq[0]->request("GET", "/echo?param=reqman0", [&](CaHttpReq &req, int event, int status) {
				if(event==req.HTTP_REQ_END) {
					ali("reqman0 response, ");
					mCode[0] = req.getRespCode();
					mRespData[0] = req.getRespData();
				}
			});

			mpReq[1] = mMan.newRequest();
			mpReq[1]->setRemoteHostAddr("127.0.0.1", SERVER_PORT);
			mpReq[1]->request("GET", "/echo?param=reqman1", [&](CaHttpReq &req, int event, int status) {
				if(event==req.HTTP_REQ_END) {
					ali("reqman1 response, ");
					mCode[1] = req.getRespCode();
					mRespData[1] = req.getRespData();
					mTask.postExit();
				}
			});

		} else if(msg.msgid == EDM_CLOSE) {
			mpReq[0]->close();
			mpReq[1]->close();
			mMan.close();
		}
		return 0;
	});
	mTask.runMain();
	quit_cahttp_server();
	ASSERT_EQ(mCode[0], 200);
	ASSERT_STREQ(mRespData[0].data(), "reqman0");
	ASSERT_EQ(mCode[1], 200);
	ASSERT_STREQ(mRespData[1].data(), "reqman1");
}

TEST(request, connection) {
	CaHttpReq mReq;
	EdTask mTask;
	mTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mReq.request_get("http://127.0.0.1:9000/echo?param=fail", [&](CaHttpReq& req, int event, int status) {
				if(req.HTTP_REQ_END) {
					ali("req end, status=%d", status);
					mTask.postExit();
				}
			});
		} else if(msg.msgid == EDM_CLOSE) {

		}
		return 0;
	});
	mTask.runMain();
}
