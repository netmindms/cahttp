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

	BaseConnection::CnnIf* getCnnIf() {
		return &mCnnIf;
	}
	std::vector<std::string>& getPathParams();
	int send(const char* ptr, size_t len);
	int response(int status_code, const char *pdata, size_t data_len, const char* ctype);

private:
	class cnnif: public BaseConnection::CnnIf {
	public:
		cnnif(ReUrlCtrl* pctrl) {
			mpCtrl = pctrl;
		}
		virtual ~cnnif() {
			;
		}
		virtual int OnWritable();
		virtual int OnMsg(std::unique_ptr<BaseMsg> upmsg);
		virtual int OnData(std::string&& data);
		virtual int OnCnn(int cnnstatus);
		ReUrlCtrl* mpCtrl;
	};

	cnnif mCnnIf;
	BaseMsg* mpReqMsg;
	BaseMsg mRespMsg;
	std::vector<std::string> mPathParams;
	BaseConnection* mCnn;
	ReSvrCnn* mpServCnn;
	uint32_t mSendHandle;
	int64_t mSendDataCnt;
	uint8_t mStatusFlag;
	int64_t mContentLen;
	std::list<std::unique_ptr<PacketBuf>> mBufList;
	edft::EdEventFd mEndEvent;
	void setPathParams(std::vector<std::string>&& vs) {
		mPathParams = move(vs);
	}
	inline uint32_t getHandle() {
		return mSendHandle;
	}
	void init(upBaseMsg upmsg, ReSvrCnn& cnn, uint32_t hsend);
	int sendHttpMsg(std::string&& msg);
	int procOnWritable();
	void stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail);
};

typedef std::unique_ptr<ReUrlCtrl> upReUrlCtrl;

} /* namespace cahttp */

#endif /* CAHTTP_REURLCTRL_H_ */
