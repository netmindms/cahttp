/*
 * server.cpp
 *
 *  Created on: Aug 7, 2015
 *      Author: netmind
 */

#define LOG_LEVEL LOG_INFO

#include "app.h"
#include <ednio/EdNio.h>
#include <cahttp/CaHttpServer.h>
using namespace edft;
using namespace std;
using namespace cahttp;

class AutoSendCtrl: public CaHttpUrlCtrl {
public:
	AutoSendCtrl(){
		mdata = "12345\n";
		mSendCnt = 0;
	};
	virtual ~AutoSendCtrl(){};
	virtual void OnHttpReqMsgHdr() override {

	};

//	virtual void OnHttpReqData(CaHttpSvrReq &req);
//	virtual void OnHttpReqDataEnd(CaHttpSvrReq &req);
	virtual void OnHttpReqMsg() override {
		ali("On req msg, task=%lx", (u64)EdTask::getCurrentTask());
		addRespHdr("X-Id", "asdfasdfasdf");
		addRespHdr(CAHS::CTYPE, "plain/text");
//		response(200, "OK\n");
		HttpStringReadStream &strm = *new HttpStringReadStream;
		strm.setString("1233456565656");
//		setRespContent(nullptr, -1);
		setRespContent(upHttpStringReadStream(&strm), -1);

		response(200);
	};

	virtual void OnHttpSendBufReady() override {
		ali("buf ready...");
		mTimer.setOnListener([this](EdTimer &timer){
			sendData(mdata.data(), mdata.size());
			if(mSendCnt++ >= 10) {
				timer.kill();
				sendData(nullptr, 0);
			}
		});
		mTimer.set(1000);
	}
//	virtual void OnHttpSendBufReady(CaHttpSvrReq &req);
	virtual void OnHttpEnd() override {
		mTimer.kill();
		ali("on server http end");
	};
private:
	string mdata;
	EdTimer mTimer;
	int mSendCnt;
};

class ManualSendCtrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {

	}

	void OnHttpEnd() override {

	}
};

class MainTask: public EdTask {
	CaHttpServer mSingleTaskServer;
	int OnEventProc(EdMsg &msg) {
		if (msg.msgid == EDM_INIT) {
			ali("start main task=%lx", (u64)EdTask::getCurrentTask());
			mSingleTaskServer.setUrl<AutoSendCtrl>("/auto_send");
//			mServer.setUrlRegEx<BasicCtrl>("/v1/([0-9]*)/([a-zA-Z0-9]*)");
			mSingleTaskServer.start(1);
			ali("start http server...");
		}
		else if (msg.msgid == EDM_CLOSE) {
			mSingleTaskServer.close();
		}
		return 0;
	}
};

#include <climits>
#include <ednio/EdRingQue.h>
#include <cahttp/RingBufReadStream.h>
int main(int argc, char* argv[]) {
//	SetLogLevel(LOG_DEBUG);
	EdNioInit();
	MainTask task;
	task.runMain();
//	getchar();
	task.terminate();
	return 0;
}
