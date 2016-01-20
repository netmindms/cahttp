
/*
 * test_server.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_INFO

#include "../cahttp/flog.h"
#include <vector>
#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/CaHttpServer.h"
#include "../cahttp/CaHttpUrlCtrl.h"
#include "../cahttp/HttpFileReadStream.h"
#include "../cahttp/CaHttpUrlParser.h"
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdio.h>

#include "test_common.h"

using namespace edft;
using namespace std;
using namespace cahttp;

static unordered_map<string, string> gResults;
static string DOWNLOAD_FILE_NAME;
static string BASE_URL="http://127.0.0.1:6060";

class _INIT_TEST_SERVER_MODULE {
public:
	_INIT_TEST_SERVER_MODULE() {
		struct utsname un;
		uname(&un);
		DOWNLOAD_FILE_NAME = string("/boot/initrd.img-") + un.release;
	}
};

static _INIT_TEST_SERVER_MODULE _this_module2;

string execCurl(const string &method, const string &url, const string& opt="") {
	string cmd = "curl -X " + method +" -s " + opt + " " + BASE_URL + url;
	auto *pst = popen(cmd.data(), "r");
	assert(pst);
	char buf[1024];
	string res;
	for(;;) {
		auto rcnt = fread(buf, 1, sizeof(buf), pst);
//		ald("file read cnt=%d", rcnt);
		if(rcnt<=0) break;
		res.append(buf, rcnt);
	}
	auto exc = pclose(pst);
	assert(exc==0);
	ald("curl exec end...");
	return move(res);
}

static string loadFileData() {
	auto fs = EdFile::getSize(DOWNLOAD_FILE_NAME);
	string ds;
	try {
		ds.resize(fs);
		EdFile file;
		file.openFile(DOWNLOAD_FILE_NAME);
		file.readFile((char*)ds.data(), ds.size());
		return move(ds);
	} catch(...) {
		return "";
	}

}

class MainUrl: public CaHttpUrlCtrl {
public:
	MainUrl() {
		ali("main const...");
	}
	;
	virtual ~MainUrl() {
		ali("main dest...");
	}
	;
	virtual void OnHttpReqMsg() override {
		addRespHdr("X-My-Header", "this app header");
		response(200, "hello", "plain/text");
	}
	;
	void OnHttpEnd() override {
		ali("main req end...");
	}

	void OnHttpSendBufReady() override {

	}

};

class ManualSendUrl: public CaHttpUrlCtrl {
public:
	string dataStr;
	ManualSendUrl() {
	}
	;
	virtual ~ManualSendUrl() {
	}
	;
	virtual void OnHttpReqMsg() override {
		ali("manual url req ...");
		dataStr = "manual-send";
		setRespContent(nullptr, dataStr.size());
		response(200);
	}
	;
	void OnHttpEnd() override {
		ali("manual url end...");
	}

	void OnHttpSendBufReady() override {
		ali("send ready...");
		sendData(dataStr.data(), dataStr.size());
	}

};

class ManualFileUrl: public CaHttpUrlCtrl {
	FILE *mSt;
	char mBuf[4*1024];
	size_t dataSize;
	size_t sendCnt;
	size_t readCnt;
	void OnHttpReqMsg() override {
		readCnt = 0;
		dataSize = 0;
		sendCnt = 0;
		mSt = fopen(DOWNLOAD_FILE_NAME.data(), "rb");
		auto fs = EdFile::getSize(DOWNLOAD_FILE_NAME);
		ASSERT_NE(mSt, nullptr);

		setRespContent(nullptr, fs);
		response(200);
	}
	void OnHttpSendBufReady() override {
//		ali("on buf ready...");
		if(dataSize==0) {
			auto rcnt = fread(mBuf, 1, sizeof(mBuf), mSt);
			if(rcnt>0) {
				readCnt += rcnt;
				dataSize=rcnt;
			} else {
				dataSize = 0;
			}
		}
		auto wret = sendData(mBuf, dataSize);
		if(!wret) {
			sendCnt += dataSize;
			dataSize = 0;
		} else {
			assert(0);
			ali("### unexpected...");
		}
	}
	void OnHttpEnd() override {
		ali("manual file end..., send cnt=%ld, rcnt=%ld", sendCnt, readCnt);
		fclose(mSt);
	}

};
class ServerExitUrl: public CaHttpUrlCtrl {
public:
	virtual void OnHttpReqMsg() override {
		ali("ServerExitUrl url req ...");
		response(200);
	}
	;
	void OnHttpEnd() override {
		ali("ServerExitUrl url end...");
		EdTask::getCurrentTask()->postExit();
	}

};

class DelayedRespUrl: public CaHttpUrlCtrl {
public:
	DelayedRespUrl() {
	}
	;
	virtual ~DelayedRespUrl() {
	}
	;
	virtual void OnHttpReqMsg() override {
		ali("DelayedResp url req ...");
		auto &urlstr = getReqUrlStr();
		CaHttpUrlParser parser;
		parser.parse(urlstr);
		unordered_map<string, string> kmap;
		parser.parseKeyValueMap(&kmap, parser.query);
		int delay=-1;
		try {
			delay = stoi(kmap["delay"]);
		} catch(...) {
			delay = -1;
		}
		if(delay<0) {
			response(403);
			return;
		}

		mTimer.setOnListener([this](EdTimer &timer) {
			timer.kill();
			response(200, "delayed-resp", "plain/text");
		});
		mTimer.set(delay);
	}
	;
	void OnHttpEnd() override {
		ali("DelayedResp url end...");
		mTimer.kill();
	}

private:
	EdTimer mTimer;
};

class ManauTransEncUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		setRespContent(nullptr, -1);
		response(200);
	}
	void OnHttpSendBufReady() override {
		string s="manual-";
		sendString(s);
		s="trans-";
		sendString(s);
		s="enc";
		sendString(s);
		sendData(nullptr, 0);
	}
	void OnHttpEnd() override {

	}

};

class ManualTransEncFileUrl: public CaHttpUrlCtrl {
	FILE *mSt;
	char mBuf[4*1024];
	size_t dataSize;
	size_t sendCnt;
	size_t readCnt;
	void OnHttpReqMsg() override {
		readCnt = 0;
		dataSize = 0;
		sendCnt = 0;
		mSt = fopen(DOWNLOAD_FILE_NAME.data(), "rb");
		ASSERT_NE(mSt, nullptr);

		setRespContent(nullptr, -1);
		response(200);
	}
	void OnHttpSendBufReady() override {
//		ali("on buf ready...");
		if(dataSize==0) {
			auto rcnt = fread(mBuf, 1, sizeof(mBuf), mSt);
			if(rcnt>0) {
				readCnt += rcnt;
				dataSize=rcnt;
			} else {
				dataSize = 0;
			}
		}
		auto wret = sendData(mBuf, dataSize);
		if(!wret) {
			sendCnt += dataSize;
			dataSize = 0;
		} else {
			assert(0);
			ali("### unexpected...");
		}
	}
	void OnHttpEnd() override {
		ali("manual trans enc file end..., send cnt=%ld, rcnt=%ld", sendCnt, readCnt);
		fclose(mSt);
	}

};

class ManualLongSendUrl: public CaHttpUrlCtrl {
	size_t mClen;
	size_t mWriteCnt;
	void OnHttpReqMsg() override {
		mClen = 10 * 1024 * 1024;
		mWriteCnt = 0;
		setRespContent(nullptr, mClen);
		response(200);
	}
	void OnHttpSendBufReady() override {
		char buf[1024];
		memset(buf, 0x55, sizeof(buf));
		auto ret = sendData(buf, sizeof(buf));
		if (ret != SEND_FAIL) {
			mWriteCnt += sizeof(buf);
			ali("write cnt=%ld", mWriteCnt);
		}
	}

	void OnHttpEnd() override {
		ali("total cnt=%d", mWriteCnt);
	}
};

class ManualPeriodicSendUrl: public CaHttpUrlCtrl {
	EdTimer mTimer;
	string mStrBuf;
	void OnHttpReqMsg() override {
		mStrBuf = "manual-periodic-send";
		setRespContent(nullptr, mStrBuf.size());
		response(200);
		mTimer.setOnListener([this](EdTimer &timer) {
			if(mStrBuf.size()>0) {
				auto c = mStrBuf.substr(0, 3);
				mStrBuf.erase(0, 3);
				sendData(c.data(), c.size());
			} else {
				timer.kill();
			}
		});
		mTimer.set(1000);
	}
	void OnHttpSendBufReady() override {
		ali("buf ready...");
	}

	void OnHttpEnd() override {
		ali("manual periodic end...");
		mTimer.kill();
	}
};

class AutoSendUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		ali("auto send req...");
		response(200, "auto-send", "application/octet-stream");
	}
	void OnHttpSendBufReady() override {
		assert(0);
	}

	void OnHttpEnd() override {
		ali("auto send end...");
	}
};

class AutoLongSendUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		string ds;
		char buf[1024];
		memset(buf, 0x55, sizeof(buf));
		for (int i = 0; i < 1024 * 10; i++) {
			ds.append(buf);
		}
		ali("data size: %ld", ds.size());
		response(200, ds, "application/octet-stream");
	}
	void OnHttpSendBufReady() override {
		assert(0);
	}

	void OnHttpEnd() override {
		ali("auto long send end...");
	}
};

class AutoFileUrl: public CaHttpUrlCtrl {
	HttpFileReadStream filestrm;
	void OnHttpReqMsgHdr() override {

	}

	void OnHttpReqMsg() override {
		struct utsname un;
		uname(&un);
//		upHttpFileReadStream strm(new HttpFileReadStream);
		string fn = string("/boot/initrd.img-") + un.release;
		filestrm.open(fn.data());
		setRespContent(&filestrm, filestrm.remain());
		addRespHdr("Content-Type", "application/octet-stream");
		response(200);
	}
	void OnHttpEnd() override {
		ali("auto file end");
	}
};

class AutoTransEncFileUrl : public CaHttpUrlCtrl {
	HttpFileReadStream filestrm;
	void OnHttpReqMsg() override {
//		upHttpFileReadStream strm(new HttpFileReadStream);
		filestrm.open(DOWNLOAD_FILE_NAME.data());
		setRespContent(&filestrm, -1);
		response(200);
	}
};


class AutoTransEncUrl : public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		setRespContent("asdfasdfasdfasdf", string("plain/text"), true);
		response(200);
	}
};

class EchoUrl : public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		auto &str = getReqUrlStr();
		CaHttpUrlParser parser;
		parser.parse(str);
		unordered_map<string, string> qm;
		parser.parseKeyValueMap(&qm, parser.query);
		auto &num = qm["id"];
		if(num=="") {
			response(403);
		} else {
			response(200, num , "plain/text");
		}
	}

	void OnHttpEnd() override {

	}
};

class ReqDefStrmUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		auto &data = getReqData();
		ald("req data size=%ld", data.size());
		if(data != "req-def-strm") {
			ale("### un-expected data: %s", data);
			assert(0);
		}
		response(200, data);
	}
	void OnHttpEnd() override {
		ali("req def strm end...");
	}
};

class FileWriteStrm : public HttpBaseWriteStream {
public:
	FileWriteStrm() {
		mSt = nullptr;
		mWriteCnt = 0;
	}
	virtual ~FileWriteStrm() {
		if(mSt) fclose(mSt);
	}
	int open(const char* fn) {
		mSt = fopen(fn, "wb");
		if(mSt) return 0;
		else return -1;
	}
	void end() override {
		if(mSt) {
			fclose(mSt); mSt = nullptr;
		}
	}

	size_t size() override {
		return mWriteCnt;
	}

	ssize_t write(const char* ptr, size_t len) override {
		auto ret = fwrite(ptr, 1, len, mSt);
		if(ret>0) {
			mWriteCnt += len;
		}
		return ret;
	}
private:
	FILE *mSt;
	size_t mWriteCnt;
};

class UploadFileUrl : public CaHttpUrlCtrl {
	FileWriteStrm mStrm;

	void OnHttpReqMsgHdr() override {
		mStrm.open("/tmp/uptest.dat");
		setReqDataStream(&mStrm);
	}

	void OnHttpReqMsg() override {
		auto dret = system(("diff /tmp/uptest.dat " + DOWNLOAD_FILE_NAME).data());
		if(!dret) {
			response(200, "upload-file");
		} else {
			response(200, "NOK");
		}

	}
};

class RegExUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		auto ps = getUrlMatchStr();
		string resp;
		for(auto &s: ps) {
			resp += s + ",";
		}
		response(200, resp, CAS::CT_TEXT_PLAIN);
	}
};

class ServerMain: public EdTask {
public:
	ServerMain() {
	}
	;
	virtual ~ServerMain() {
	}
	;

	int OnEventProc(EdMsg &msg) {
		if (msg.msgid == EDM_INIT) {
			mServer.config("port", "6060");
			mServer.config("ip", "0.0.0.0");
			mServer.setUrl<MainUrl>(HTTP_GET, "/main");
			mServer.setUrl<ManualSendUrl>(HTTP_GET, "/manual-send");
			mServer.setUrl<ManualFileUrl>(HTTP_GET, "/manual-file");
			mServer.setUrl<ManualLongSendUrl>(HTTP_GET, "/manual-long-send");
			mServer.setUrl<ManualPeriodicSendUrl>(HTTP_GET, "/manual-periodic-send");
			mServer.setUrl<ManauTransEncUrl>(HTTP_GET, "/manual-trans-enc");
			mServer.setUrl<ManualTransEncFileUrl>(HTTP_GET, "/manual-trans-enc-file");
			mServer.setUrl<DelayedRespUrl>(HTTP_GET, "/delayed-resp");
			mServer.setUrl<AutoSendUrl>(HTTP_GET, "/auto-send");
			mServer.setUrl<AutoFileUrl>(HTTP_GET, "/auto-file");
			mServer.setUrl<AutoTransEncUrl>(HTTP_GET, "/auto-trans-enc");
			mServer.setUrl<AutoTransEncFileUrl>(HTTP_GET, "/auto-trans-enc-file");
			mServer.setUrl<EchoUrl>(HTTP_GET, "/echo");
			mServer.setUrl<ReqDefStrmUrl>(HTTP_POST, "/req-def-strm");
			mServer.setUrl<UploadFileUrl>(HTTP_POST, "/upload-file");
			mServer.setUrl<ServerExitUrl>(HTTP_GET, "/server-exit");
			mServer.setUrlRegEx<RegExUrl>(HTTP_GET, "/regex/([a-z]*)/([0-9]*)");
			mServer.start(0);
		} else if (msg.msgid == EDM_CLOSE) {
			mServer.close();
		}
		return 0;
	}
private:
	CaHttpServer mServer;
};

TEST(server, manualsend) {
	ServerMain mainTask;
	mainTask.run();
	auto resp = execCurl("GET", "/manual-send");
	ASSERT_STREQ(resp.data(), "manual-send");

	resp = execCurl("GET", "/manual-periodic-send");
	ASSERT_STREQ(resp.data(), "manual-periodic-send");

	resp = execCurl("GET", "/manual-trans-enc");
	ASSERT_STREQ(resp.data(), "manual-trans-enc");

	auto filedata = loadFileData();
	resp = execCurl("GET", "/manual-file");
	ASSERT_EQ(resp==filedata, true);

	mainTask.terminate();
}

TEST(server, autosend) {
	ServerMain mainTask;
	mainTask.run();

	string resp;

//	resp = execCurl("GET", "/auto-send");
//	ASSERT_STREQ(resp.data(), "auto-send");
//
//
	string filedata = loadFileData();
//	resp = execCurl("GET", "/auto-file");
//	ASSERT_EQ((resp==filedata), true);

	resp = execCurl("GET", "/auto-trans-enc-file");
	ASSERT_EQ((resp==filedata), true);

	mainTask.terminate();
}

TEST(server, multi) {
	ServerMain mainTask;
	mainTask.run();
	string filedata = loadFileData();

	vector<thread> ths;
	ths.emplace_back([&](){
		string resp = execCurl("GET", "/auto-file");
		ASSERT_EQ((resp==filedata), true);
		;
	});
	ths.emplace_back([&](){
		string resp = execCurl("GET", "/manual-file");
		ASSERT_EQ((resp==filedata), true);
		;
	});
	ths.emplace_back([&](){
			string resp = execCurl("GET", "/manual-trans-enc-file");
			ASSERT_EQ((resp==filedata), true);
			;
		});
	ths.emplace_back([&](){
			string resp = execCurl("GET", "/auto-trans-enc-file");
			ASSERT_EQ((resp==filedata), true);
			;
		});
	ths.emplace_back([&](){
		string resp = execCurl("GET", "/auto-send");
		ASSERT_EQ((resp=="auto-send"), true);
		;
	});
	ths.emplace_back([&](){
		string resp = execCurl("GET", "/manual-send");
		ASSERT_EQ((resp=="manual-send"), true);
		;
	});
	for(auto &t: ths) {
		t.join();
	}
	mainTask.terminate();
}

TEST(server, pipeline) {
	ServerMain mainTask;
	mainTask.run();

	EdTask task;
	EdSocket clsock;
	string resp;
	size_t p1, p2;
	task.setOnListener([&](EdMsg &msg) -> int {
		if(msg.msgid == EDM_INIT) {
			clsock.setOnListener([&](EdSocket &sock, int event) {
				if(event == SOCK_EVENT_CONNECTED) {
					string dm ="GET /delayed-resp?delay=1000 HTTP/1.1\r\n\r\n";
					string as = "GET /auto-send HTTP/1.1\r\n\r\n";
					string ts = dm+as;
					sock.send(ts.data(), ts.size());
				} else if(event == SOCK_EVENT_DISCONNECTED) {
					ASSERT_EQ(1, 0);
				} else if(event == SOCK_EVENT_READ) {
					char buf[500];
					auto rcnt = sock.recv(buf, 100);
					if(rcnt>0) resp.append(buf, rcnt);
				}
			});
			clsock.connect("127.0.0.1", 6060);
			task.setTimer(1, 2000);
		} else if(msg.msgid == EDM_CLOSE) {
			clsock.close();
//			ali("resp=\n%s", resp);
			p1 = resp.find("delayed-resp");
			p2 = resp.find("auto-send");
		} else if(msg.msgid == EDM_TIMER) {
			task.killTimer(1);
			task.postExit();
		}
		return 0;
	});
	task.runMain();
	ASSERT_GT(p2, p1);
	mainTask.terminate();
}

TEST(server, tec) {
	ServerMain mainTask;
	mainTask.run();
	auto res = execCurl("GET", "/manual-trans-enc");
	ASSERT_STREQ(res.data(), "manual-trans-enc");

	EdFile file;
	file.openFile(DOWNLOAD_FILE_NAME);
	auto fs = file.getSize(DOWNLOAD_FILE_NAME);
	char *buf = new char[fs];
	file.readFile(buf, fs);

	res = execCurl("GET", "/auto-trans-enc-file");
	ASSERT_EQ( memcmp(res.data(), buf, res.size()), 0);

	res = execCurl("GET", "/manual-trans-enc-file");
	ASSERT_EQ( memcmp(res.data(), buf, res.size()), 0);

	delete[] buf;

	mainTask.terminate();
}

TEST(server, basic) {
	ServerMain mainTask;
	mainTask.run();

	string resp;

	resp = execCurl("GET", "/delayed-resp?delay=500");
	ASSERT_EQ(resp=="delayed-resp", true);

	mainTask.terminate();
}


TEST(server, reqdata) {
	ServerMain mainTask;
	FDCHK_S(0);
	mainTask.run();
	string resp;
	resp = execCurl("POST", "/req-def-strm", "-d 'req-def-strm'");
	ASSERT_STREQ(resp.data(), "req-def-strm");
	string opt = " -H 'Content-Type: application/octet-stream' ";
	opt += "--data-binary @" + DOWNLOAD_FILE_NAME;

	resp = execCurl("POST", "/upload-file", opt);
	ASSERT_STREQ(resp.data(), "upload-file");

	mainTask.terminate();
	FDCHK_E(0);

}

#ifdef CAHTTP_REGEX_URLPATTERN
TEST(server, regex) {
	ServerMain mainTask;
	mainTask.run();

	string resp;

	resp = execCurl("GET", "/regex/ABC/012");
	ASSERT_EQ(resp.empty(), true);

	resp = execCurl("GET", "/regex/abc/012");
	ASSERT_EQ(resp=="abc,012,", true);
	mainTask.terminate();
}
#endif

void serverlive() {
	ServerMain mainTask;
	mainTask.runMain();
}
