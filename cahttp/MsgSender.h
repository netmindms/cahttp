/*
 * MsgSender.h
 *
 *  Created on: Feb 27, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_MSGSENDER_H_
#define CAHTTP_MSGSENDER_H_

#include <ednio/EdEventFd.h>
#include "CaHttpCommon.h"
#include "BaseCnn.h"
#include "CaHttpCommon.h"
#include "BaseMsg.h"
#include "PacketBuf.h"

namespace cahttp {

class MsgSender {
public:
	enum TR {
		eMsgSendOk,
		eMsgContinue,
		eMsgSendFail,
		eMsgDataNeeded,
	};
	MsgSender();
	virtual ~MsgSender();
	int open(BaseCnn& cnn);
	int sendMsg(BaseMsg& msg);
//	SR sendData(const char* ptr, size_t len);
	SR sendData(const char* ptr, size_t len, bool buffering);
	int sendContent(const char* ptr, size_t len);
	int sendContent(const std::string& data);
	int sendContent(std::unique_ptr<PacketBuf> upbuf);
	int sendContentFile(const char* path);
	void endData();
	void close();
	inline int getTxChannel() const {
		return mTxChannel;
	}
	void reserveWrite();
	TR procOnWritable();
private:
	union status_t {
		uint8_t val;
		struct {
			uint8_t used: 1;
			uint8_t te: 1; // ransfer encoding
			uint8_t se: 1; // sending end
			uint8_t phase: 1; // 0: msg sending, 1: data sending phase
			uint8_t final: 1;
		};
	};
	uint32_t mTxChannel;
	BaseCnn* mpCnn;
	status_t mStatus;
	std::list<std::unique_ptr<PacketBuf>> mBufList;
	int64_t mSendDataCnt;
	int64_t mRecvDataCnt;
	int64_t mContentLen;
	std::string mRecvDataBuf;

	void forceCloseChannel() {
//		mpCnn->removeTxChannel(mTxChannel);
//		mTxChannel = 0;
	};

	void stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail, bool front);
	void stackSendBuf(std::string&& s, int type);
	void stackSendBuf(const char* ptr, size_t len, int type);

};

} /* namespace cahttp */

#endif /* CAHTTP_MSGSENDER_H_ */
