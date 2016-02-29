/*
 * MsgTransmitter.h
 *
 *  Created on: Feb 27, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_MSGTRANSMITTER_H_
#define CAHTTP_MSGTRANSMITTER_H_

#include <ednio/EdEventFd.h>
#include "CaHttpCommon.h"
#include "BaseConnection.h"
#include "CaHttpCommon.h"
#include "BaseMsg.h"
#include "PacketBuf.h"

namespace cahttp {

class MsgTransmitter {
public:
	enum TR {
		eSendOk,
		eSendFail,
		eDataNeeded,
	};
	MsgTransmitter();
	virtual ~MsgTransmitter();
	int open(BaseConnection& cnn, bool firstch, std::function<void(TR)> lis);
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
private:
	union status_t {
		uint8_t val;
		struct {
			uint8_t used: 1;
			uint8_t te: 1;
			uint8_t se: 1; // sending end
			uint8_t phase: 1; // 0: msg sending, 1: data sending phase
			uint8_t final: 1;
		};
	};
	uint32_t mTxChannel;
	BaseConnection* mpCnn;
	std::function<void(TR)> mLis;
	status_t mStatus;
	std::list<std::unique_ptr<PacketBuf>> mBufList;
	int64_t mSendDataCnt;
	int64_t mRecvDataCnt;
	int64_t mContentLen;
	std::string mRecvDataBuf;

	int procOnWritable();
	void stackTeByteBuf(const char* ptr, size_t len, bool head, bool body, bool tail, bool front);
	void stackSendBuf(std::string&& s, int type);
	void stackSendBuf(const char* ptr, size_t len, int type);

};

} /* namespace cahttp */

#endif /* CAHTTP_MSGTRANSMITTER_H_ */
