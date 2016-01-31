/*
 * HttpMsgFrame22.h
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */

#ifndef SRC_CAHTTPFRAME2_H_
#define SRC_CAHTTPFRAME2_H_

#include <vector>
#include <list>
#include <utility>
#include <climits>
#include "BaseMsg.h"
#include "http_parser.h"

namespace cahttp {


class HttpMsgFrame2
{
	enum PARSER_STATUS_E
	{
		PS_INIT, PS_FIRST_LINE, PS_HEADER, PS_BODY, PS_MP_HEADER, PS_MP_DATA, PS_END,
	};

	enum SEND_RESULT_E
	{
		HTTP_SEND_FAIL = -1, HTTP_SEND_OK = 0, HTTP_SEND_PENDING,
	};

	friend class _HTTP_MSG_FRAME2_MODULE_INITIALIZER;
public:
	enum {
		FS_IDLE,
		FS_CONT,
		FS_NONE,
		FS_HDR,
		FS_DATA,
//		FS_END,
	};
	enum FETCH_RESULT {
		MSG_NONE=-1,
		MSG_ONLY,
		MSG_WITHDATA,
		MSG_DATA,
		MSG_DATA_EMPTY,
		MSG_DATA_END,
		FETCH_ERROR,
	};
	HttpMsgFrame2();
	virtual ~HttpMsgFrame2();
	int init(bool isreq);
	size_t feedPacket(vector<char> &&pkt);
	size_t feedPacket(const char* buf, size_t len);
//	CaHttpMsg fetchMsg();
	int fetchMsg(BaseMsg& msg);
//	string fetchData();
	int fetchData(std::string& data);

	int status() const;
	void recycleDataBuf(std::string &&bufstr);
	void clear();
	void setHost(const std::string& hostname);
	void setUserAgent(const std::string& agent);
	void frameMsg(BaseMsg& msg);
	void frameData(std::string &&data);
	std::string pullPacket();
	bool isEmptyPacket();

private:
	bool mIsReq;
//	int mStatus;
	http_parser mParser;
	PARSER_STATUS_E mPs;
	std::string mCurHdrName, mCurHdrVal, mCurUrl;
	std::string mBodyData;

	std::string mHost;
	std::string mAgent;
	std::string mEncPkt;
	std::unique_ptr<BaseMsg> mMsg;
	int mFrameStatus;
	int64_t mContentLen;
	int64_t mReadLen;
	uint8_t mReadStatus;
	uint32_t mMsgSeqNum;

private:
	struct MsgSeq {
		uint32_t seq;
		std::unique_ptr<BaseMsg> msg;
	};
	struct DataSeq {
		uint32_t seq;
		std::string data;
	};

	void initHttpParser();
	void procFirstLine();
	void procHeader();

	static int head_field_cb(http_parser* parser, const char *at, size_t length);
	static int head_val_cb(http_parser* parser, const char *at, size_t length);
	static int body_cb(http_parser* parser, const char *at, size_t length);
	static int msg_begin(http_parser* parser);
	static int msg_end(http_parser* parser);
	static int on_url(http_parser* parser, const char* at, size_t length);
	static int on_headers_complete(http_parser* parser);
	static int on_status(http_parser* parser, const char* at, size_t length);
	static int chunk_header_cb(http_parser* parser);
	static int chunk_comp_cb(http_parser* parser);
	int dgHeaderNameCb(http_parser*, const char* at, size_t length);
	int dgHeaderValCb(http_parser*, const char* at, size_t length);
	int dgHeaderComp(http_parser* parser);
	int dgbodyDataCb(http_parser* parser, const char* at, size_t length);
	int dgMsgBeginCb(http_parser* parser);
	int dgMsgEndCb(http_parser* parser);
	int dgUrlCb(http_parser* parser, const char* at, size_t length);
	int dgFirstLineStatus(http_parser* parser, const char* at, size_t length);

	std::list<MsgSeq> mMsgList;
	std::list<DataSeq> mDataList;

};
}
#endif /* SRC_CAHTTPFRAME_H_ */
