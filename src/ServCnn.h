/*
 * ServCnn.h
 *
 *  Created on: Jul 19, 2015
 *      Author: netmind
 */

#ifndef SRC_SERVCNN_H_
#define SRC_SERVCNN_H_

#include <list>
#include <memory>
#include <ednio/EdNio.h>
#include "CaHttpCommon.h"
#include "CaHttpFrame.h"
#include "CaHttpUrlCtrl.h"
#include "HttpMsgStream.h"
namespace cahttp {
class ServTask;
class HttpServCnnCtx;

class ServCnn {
	friend class CaHttpUrlCtrl;
public:
	ServCnn();
	ServCnn(const ServCnn&);
	ServCnn(int handle);
	virtual ~ServCnn();
	int open(int fd, uint32_t handle, HttpServCnnCtx* pctx);
	void close();
	void setUserObj(upCaUserObj obj) {
		mUserObj = move(obj);
	};
	template<class T> T* getUserObj() {
		auto ptr = mUserObj.get();
		return dynamic_cast<T*>(ptr);
	};
private:
	HttpServCnnCtx *mpCtx;
	EdSmartSocket mSock;
	CaHttpFrame mMsgFrame;
	char *mRecvBuf;
	size_t mBufSize;
//	ServTask *mTask;
	CaHttpUrlCtrl* mReqCtrl;
	CaHttpUrlCtrl *mRespCtrl;
	uint32_t mHandle;
	EdTimer mCnnTimer;
	int mTimerWork;
	bool mIsPipeliningSupport;
	bool mParsingStop;
	upCaUserObj mUserObj;
	int mTimerUpdate;
	std::list<CaHttpUrlCtrl*> mCtrlList;
	std::list<CaHttpUrlCtrl*> mDummyCtrlList;
	bool mPauseSend;
	edft::EdTaskMsgQue mLocalMsg;

	void init_local_msgque();
	void procRead();
	void transfer(CaHttpUrlCtrl* pctrl);
	void starttx();
	int writeData(CaHttpUrlCtrl &ctrl, const char* ubf, size_t len);
//	CaHttpUrlCtrl* findAndAlloc(const std::string& urlstr);
	void gotoDummyCtrl(CaHttpUrlCtrl *pctrl);
	void endCtrl(CaHttpUrlCtrl *ctrl);
	void clearDummyCtrl();
	void reserveWrite();
	void postCloseConnection();
};
}
#endif /* SRC_SERVCNN_H_ */
