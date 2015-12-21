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
#include <climits>
#include <iostream>
#include <cahttp/HttpFileReadStream.h>
#include "FileUtil.h"

using namespace edft;
using namespace std;
using namespace cahttp;

struct App {
	App() {
		port = 9000;
		rootDir = "/usr/lib";
	}
	int port;
	string rootDir;
};

App gApp;

class ListFileUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		string resp;
		resp= "<html><head></head><body>";
		string ls;
		auto fl = FileUtil::getFileList(gApp.rootDir.data(), FileUtil::DIR_ONLY);
		ls = "<ul style='list-style-type:none'>";
		for(auto &f: fl) {
			ls += "<li style='color:blue'><b><u><a href=/file/dir/" + f+ ">"+f+"</a></u></b></li>\n";
		}
		fl = FileUtil::getFileList(gApp.rootDir.data(), FileUtil::FILE_ONLY);
		for(auto &f: fl) {
			ls += "<li>" + f+ "</li>\n";
		}
		ls += "</ul>\n";
		resp += ls;
		resp += "</body></html>\n";

		response(200, resp, CAS::CT_TEXT_HTML);
	}
};


class DirUrl: public CaHttpUrlCtrl {
	void OnHttpReqMsg() override {
		auto mph = getUrlMatchStr();

		string dirpath = gApp.rootDir;
		for(auto &ps: mph) {
			dirpath += ps;
		}

		string resp;
		resp= "<html><head></head><body>";
		string ls;
		auto fl = FileUtil::getFileList(dirpath.data(), FileUtil::DIR_ONLY);
		ls = "<ul style='list-style-type:none'>";
		for(auto &f: fl) {
			ls += "<li style='color:blue'><b><u><a href=/file/dir" + mph[0]+ f+"/>"+f+"</a></u></b></li>\n";
		}
		fl = FileUtil::getFileList(dirpath.data(), FileUtil::FILE_ONLY);
		for(auto &f: fl) {
			ls += "<li><a href=/file/download"+ mph[0]+ f+ ">" + f+  "</a></li>\n";
		}
		ls += "</ul>\n";
		resp += ls;
		resp += "</body></html>\n";

		response(200, resp, CAS::CT_TEXT_HTML);
	}
};

class DownloadUrl: public CaHttpUrlCtrl {
	HttpFileReadStream mFileStrm;
	void OnHttpReqMsg() override {
		auto &vs = getUrlMatchStr();
		if(vs.empty()) {
			response(404, "File Not Found");
			return;
		}
		string path;
		path = gApp.rootDir+"/"+vs[0];
		if(FileUtil::isExist(path)) {
			mFileStrm.open(path.data());
			addRespHdr(CAS::HS_CONTENT_TYPE, CAS::CT_APP_OCTET);
			setRespContent(&mFileStrm, (int64_t)mFileStrm.remain());
			response(200);
		} else {
			response(404, "File Not Found");
			return;
		}

		string downpath = gApp.rootDir+"/"+ path;
		mFileStrm.open(downpath.data());
	}

	void OnHttpEnd() override {
		mFileStrm.close();
	}
};
class MainTask: public EdTask {
	CaHttpServer mServer;
	int OnEventProc(EdMsg &msg) {
		if (msg.msgid == EDM_INIT) {
			mServer.config("port", to_string(gApp.port).data());
			mServer.setUrl<ListFileUrl>(HTTP_GET, "/file/list");
			mServer.setUrlRegEx<DirUrl>(HTTP_GET, "/file/dir(/[^\\?]*)");
//			mServer.setUrlRegEx<DirUrl>(HTTP_GET, "/file/dir(/[^\\?]*)(.*)");
			mServer.setUrlRegEx<DownloadUrl>(HTTP_GET, "/file/download/(.*)");
			mServer.start(0);
		}
		else if (msg.msgid == EDM_CLOSE) {
			mServer.close();
		}
		return 0;
	}
};


int main(int argc, char* argv[]) {
	EdNioInit();
	MainTask task;
	task.runMain();
	return 0;
}
