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
#include "MsgTransmitter.h"
#include "PacketBuf.h"
namespace cahttp {

class ReSvrCnn;

class ReUrlCtrl {
	friend class ReHttpServer;
	friend class ReHttpSvrCtx;
	friend class ReSvrCnn;
private:
	union status_t {
		uint8_t val;
		struct {
			uint8_t used: 1;
			uint8_t te: 1;
			uint8_t se: 1; // sending end
			uint8_t fin: 1; // req finished
			uint8_t err: 4;
		};
	};
public:

	ReUrlCtrl();
	virtual ~ReUrlCtrl();
	virtual void OnHttpReqMsgHdr(BaseMsg& msg);
	virtual void OnHttpReqMsg(BaseMsg& msg);
	virtual void OnHttpReqData(std::string&& data);
	virtual void OnHttpSendBufReady();
	virtual void OnHttpEnd(int err);

	std::vector<std::string>& getPathParams();
	SR send(const char* ptr, size_t len, bool buffering=false);
	int response(int status_code, const char *pdata, size_t data_len, const char* ctype);
	int response(BaseMsg& msg);
	int response(int status_code);
	int response(int status_code, const std::string& content, const std::string& ctype);
	int response_file(const char* path);
	int writeContent(const char* ptr, size_t len);
	int sendContent(const char* ptr, size_t len);
	void endData();
	void setTransferEncoding(bool te) {
		mRespMsg.setTransferEncoding(te);
	};
	inline int getErr() {
		return mStatus.err;
	}
	inline void clearMsg() {
		mRespMsg.clear();
	}
private:
	upBaseMsg mupReqMsg;
	BaseMsg mRespMsg;
	std::vector<std::string> mPathParams;
	BaseConnection* mpCnn;
	ReSvrCnn* mpServCnn;
	uint32_t mHandle;
	int64_t mSendDataCnt;
	int64_t mRecvDataCnt;
	status_t mStatus;
	int64_t mContentLen;
	std::string mRecvDataBuf;
	uint32_t mRxChannel;
	MsgTransmitter mMsgTx;

//	std::list<std::unique_ptr<PacketBuf>> mBufList;
//	edft::EdEventFd mEndEvent;
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
	void stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail, bool front);
	void stackSendBuf(std::string&& s, int type);
	void stackSendBuf(const char* ptr, size_t len, int type);
	void setBasicHeader(BaseMsg& msg, int status_code);
	bool isComplete();
	void closeChannels();
};

typedef std::unique_ptr<ReUrlCtrl> upReUrlCtrl;

} /* namespace cahttp */

#endif /* CAHTTP_REURLCTRL_H_ */
