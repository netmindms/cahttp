/*
 * ReUrlCtrl.h
 *
 *  Created on: Feb 16, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_REURLCTRL_H_
#define CAHTTP_REURLCTRL_H_

#include <string>
#include <vector>
#include <functional>
#include <ednio/EdEventFd.h>
#include "CaHttpCommon.h"
#include "BaseConnection.h"
#include "BaseMsg.h"
#include "PacketBuf.h"
namespace cahttp {

class ReSvrCnn;

class ReUrlCtrl {
	friend class ReHttpServer;
	friend class ReHttpSvrCtx;
	friend class ReSvrCnn;
public:

	ReUrlCtrl();
	virtual ~ReUrlCtrl();
	virtual void OnHttpReqMsgHdr(BaseMsg& msg);
	virtual void OnHttpReqMsg(BaseMsg& msg);
	virtual void OnHttpReqData(std::string&& data);
	virtual void OnHttpSendBufReady();
	virtual void OnHttpEnd();

	std::vector<std::string>& getPathParams();
	int send(const char* ptr, size_t len);
	int response(int status_code, const char *pdata, size_t data_len, const char* ctype);
	int response(BaseMsg& msg);
	int response(int status_code);
	int response(int status_code, const std::string& content, const std::string& ctype);
	int response_file(int status_code, const char* path);
	int writeContent(const char* ptr, size_t len);
private:
	BaseMsg* mpReqMsg;
	BaseMsg mRespMsg;
	std::vector<std::string> mPathParams;
	BaseConnection* mCnn;
	ReSvrCnn* mpServCnn;
	uint32_t mHandle;
	int64_t mSendDataCnt;
	int64_t mRecvDataCnt;
	uint8_t mStatusFlag;
	int64_t mContentLen;
	std::string mRecvDataBuf;

	std::list<std::unique_ptr<PacketBuf>> mBufList;
	edft::EdEventFd mEndEvent;
	void setPathParams(std::vector<std::string>&& vs) {
		mPathParams = move(vs);
	}
	inline uint32_t getHandle() {
		return mHandle;
	}
	void init(upBaseMsg upmsg, ReSvrCnn& cnn, uint32_t hsend);
	int sendHttpMsg(std::string&& msg);
	int procOnWritable();
	int procOnData(std::string&& data);
	void stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail);
	void stackSendBuf(std::string&& s);
	void stackSendBuf(const char* ptr, size_t len);
	void setBasicHeader(BaseMsg& msg, int status_code);
	bool isComplete();
};

typedef std::unique_ptr<ReUrlCtrl> upReUrlCtrl;

} /* namespace cahttp */

#endif /* CAHTTP_REURLCTRL_H_ */
