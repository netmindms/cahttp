/*
 * httpclient.cpp
 *
 *  Created on: Aug 6, 2015
 *      Author: netmind
 */
#include <chrono>

#include <cahttp/flog.h>
#include <iostream>
#include <list>
#include <vector>

#include <ednio/EdNio.h>
#include <cahttp/HttpCnn.h>


using namespace std;
using namespace std::chrono;
using namespace edft;
using namespace cahttp;

#define NOW() system_clock::now()

struct App {
	string req_url;
};

App gApp;

class MainTask: public EdTask {
	CaHttpReq mReq;
	string mRespData;
	int mStatusCode;
	int OnEventProc(EdMsg& msg) override {
		if(msg.msgid == EDM_INIT) {
			mReq.request_get(gApp.req_url, [this](CaHttpReq &req, int event, int status) {
				if(event == req.HTTP_REQ_END) {
					if(!status) {
						mStatusCode = req.getRespCode();
						mRespData = req.getRespData();
						printf("%s", mRespData.data());
						fprintf(stderr, "status code=%d\n", mStatusCode);
						postExit();
					}
				}
			});
		}
		else if(msg.msgid == EDM_CLOSE) {
			mReq.close();
		}
		return 0;
	}
};

int main(int argc, char* argv[]) {
	EdNioInit();
	if(argc>1) {
		gApp.req_url = argv[1];
	} else {
		exit(1);
	}

	MainTask task;
	task.runMain();
	return 0;
}


