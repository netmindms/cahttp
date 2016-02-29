/*
 * test_svr2.cpp
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */



#define LOG_LEVEL LOG_DEBUG

#include <gtest/gtest.h>
#include <ednio/EdNio.h>
#include "../cahttp/ReHttpServer.h"
#include "../cahttp/flog.h"
#include "../cahttp/CaHttpCommon.h"
#include "../cahttp/ReUrlCtrl.h"
#include "testutil.h"

using namespace cahttp;
using namespace edft;
using namespace std;

namespace _svr2 {
class ServerTask;


class SvrTestUrlCtr: public ReUrlCtrl {
public:
	SvrTestUrlCtr(ServerTask *ptask) {
		mpSvrTask = (EdTask*)ptask;
	}
	virtual ~SvrTestUrlCtr() {

	}
	virtual void OnHttpReqMsg(BaseMsg& msg) override {
		ali("on http reqmsg");
	};

	virtual void OnHttpReqData(string&& data) override {
		ali("req data: |%s|", data);
	};
	virtual void OnHttpEnd(int err) override {
		ali("on http end, err=%d", err);
	};
protected:
	EdTask *mpSvrTask;
};

class EchoUrl: public SvrTestUrlCtr {
public:
	EchoUrl(ServerTask* ptask): SvrTestUrlCtr(ptask)  {
		ali("echo url const");
	}
	string mRecvStr;
	virtual void OnHttpReqMsg(BaseMsg& msg) override {
		ali("echo req msg...");
		auto r = response(200, mRecvStr, CAS::CT_APP_OCTET);
		assert(r==0);
	};
	virtual void OnHttpReqData(string&& data) override {
		mRecvStr += data;
	}
	virtual void OnHttpEnd(int err) override {
		ald("echo req end. err=%d", err);
	};
};

class TaskExit: public SvrTestUrlCtr {
public:
	TaskExit(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		mpSvrTask->postExit();
		response(200);
	}
};

class Hello: public SvrTestUrlCtr {
public:
	Hello(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		ali("hello request");
		response(200);
	}
	void OnHttpEnd(int err) override {
		ali("hello end, err=%d", err);
	}
};


class NoResponse: public SvrTestUrlCtr {
public:
	NoResponse(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		ali("noreponse request");
	}
	void OnHttpEnd(int err) override {
		ali("noreponse end, err=%d", err);
	}
};



class DelayResponse: public SvrTestUrlCtr {
public:
	EdTimer mTimer;
	DelayResponse(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		ali("delay request");
		mTimer.setOnListener([this]() {
			mTimer.kill();
			response(200, "1sec", CAS::CT_TEXT_PLAIN);
		});
		mTimer.set(1000);
	}
	void OnHttpEnd(int err) override {
		ali("delay response end, err=%d", err);
	}
};

class FileResponse: public SvrTestUrlCtr {
public:
	FileResponse(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		ali("file request");
		setTransferEncoding(true);
		auto r = response_file(get_test_file_path().data());
//		auto r = response_file("/home");
//		auto r = response_file("/mnt/hgfs/share/bsp.tar.gz");
		assert(!r);
	}
	void OnHttpEnd(int err) override {
		ali("file end, err=%d", err);
	}
};


class ProvisioningResp: public SvrTestUrlCtr {
public:
	EdTimer mTimer;
	ProvisioningResp(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		ali("provisioing request");
		response(100);
		mTimer.setOnListener([this]() {
			mTimer.kill();
			clearMsg();
			response(200, "final response\n", CAS::CT_APP_OCTET);
		});
		mTimer.set(1000);
	}
	void OnHttpEnd(int err) override {
		ali("provisioing req end, err=%d", err);
	}
};

class ProvisioningDataResp: public SvrTestUrlCtr {
public:
	EdTimer mTimer;
	ProvisioningDataResp(ServerTask* ptask): SvrTestUrlCtr(ptask) {

	};
	void OnHttpReqMsg(BaseMsg& msg) override {
		ali("provisioning request with data");
		response(100, "this is 100 message...\n", CAS::CT_APP_OCTET);
		mTimer.setOnListener([this]() {
			mTimer.kill();
			clearMsg();
			response(200, "final response\n", CAS::CT_APP_OCTET);
		});
		mTimer.set(1000);
	}
	void OnHttpEnd(int err) override {
		ali("provisioing data req end, err=%d", err);
	}
};

class ServerTask: public EdTask {
	ReHttpServer mSvr;
	int OnEventProc(EdMsg& msg) override {
		if(msg.msgid == EDM_INIT) {
			mSvr.setUrlReg<NoResponse>(HTTP_GET, "/noresponse", this);
			mSvr.setUrlReg<Hello>(HTTP_GET, "/hello", this);
			mSvr.setUrlReg<TaskExit>(HTTP_GET, "/exit", this);
			mSvr.setUrlReg<EchoUrl>(HTTP_POST, "/echo", this);
			mSvr.setUrlReg<FileResponse>(HTTP_GET, "/file", this);
			mSvr.setUrlReg<DelayResponse>(HTTP_GET, "/delay", this);
			mSvr.setUrlReg<ProvisioningResp>(HTTP_GET, "/provisioning", this);
			mSvr.setUrlReg<ProvisioningDataResp>(HTTP_GET, "/provisioning_data", this);
			mSvr.start(0);
		} else if(msg.msgid == EDM_CLOSE) {
			ald("task closing...");
			mSvr.close();
		}
		return 0;
	}
};

}

TEST(svr2, basic) {
	_svr2::ServerTask svrTask;
	svrTask.runMain();
}

#if 0
TEST(svr2, svr) {
//	ReHttpServer::test();
	class TestUrl: public ReUrlCtrl {
		string strBuf;
		virtual void OnHttpReqMsg(BaseMsg& msg) override {
			ali("on req message");
//			response_file(200, "/home/netmind/temp/h");
			response(200, "hello", CAS::CT_TEXT_PLAIN);
//			response(200, nullptr, 10, CAS::CT_TEXT_PLAIN.data());
		};
		virtual void OnHttpReqData(std::string&& data) override {
			ali("on req data");
			strBuf += data;
		};
		virtual void OnHttpEnd() override {
			ali("on req end");
			ali("  recv data=%s", strBuf);
		};
	};
	EdTask mTask;
	ReHttpServer mSvr;
	EdTimer mTimer;
	mTask.setOnListener([&](EdMsg &msg) {
		if(msg.msgid == EDM_INIT) {
			mSvr.setUrlReg<TestUrl>(HTTP_GET, "/test");
			mSvr.setUrlReg<EchoUrl>(HTTP_POST, "/echo");
			mSvr.start(0);
		} else if(msg.msgid == EDM_CLOSE) {
			mTimer.kill();
			mSvr.close();
		}
		return 0;
	});
	mTask.runMain();
}

#endif
