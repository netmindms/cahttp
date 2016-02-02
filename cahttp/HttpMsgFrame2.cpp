/*
 * HttpMsgFrame2.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_WARN
#include <climits>
#include "flog.h"
#include "HttpMsgFrame2.h"

namespace cahttp {
static http_parser_settings _parserSettings;

class _HTTP_MSG_FRAME2_MODULE_INITIALIZER {
public:
	_HTTP_MSG_FRAME2_MODULE_INITIALIZER() {
		_parserSettings.on_message_begin = HttpMsgFrame2::msg_begin;
		_parserSettings.on_message_complete = HttpMsgFrame2::msg_end;
		_parserSettings.on_url = HttpMsgFrame2::on_url;
		_parserSettings.on_status = HttpMsgFrame2::on_status;
		_parserSettings.on_header_field = HttpMsgFrame2::head_field_cb;
		_parserSettings.on_header_value = HttpMsgFrame2::head_val_cb;
		_parserSettings.on_headers_complete = HttpMsgFrame2::on_headers_complete;
		_parserSettings.on_body = HttpMsgFrame2::body_cb;
		_parserSettings.on_chunk_header = HttpMsgFrame2::chunk_header_cb;
		_parserSettings.on_chunk_complete = HttpMsgFrame2::chunk_comp_cb;
	}

};

static _HTTP_MSG_FRAME2_MODULE_INITIALIZER _mod_init;

HttpMsgFrame2::HttpMsgFrame2() {
	mMsg = nullptr;
	mIsReq = false;
//	mStatus = 0;
	mPs = PS_INIT;
	mFrameStatus = FS_IDLE;
	mReadLen = 0;
	mReadStatus = 0;
	mContentLen = 0;
	mMsgSeqNum = 0;
}

HttpMsgFrame2::~HttpMsgFrame2() {
}

int HttpMsgFrame2::fetchMsg(BaseMsg& msg) {
	if(!mMsgList.empty()) {
		assert(mDataList.empty() || mMsgList.front().seq <= mDataList.front().seq);
		auto &msgseq = mMsgList.front();
		auto mseqnum = msgseq.seq;
		msg = move((*msgseq.msg));
		uint32_t dseq=UINT32_MAX;
		if(mDataList.empty()==false) {
			dseq = mDataList.front().seq;
		} else if(mBodyData.empty()==false) {
			dseq = mMsgSeqNum;
		}
		mMsgList.pop_front();
		if(dseq == mseqnum) {
			return MSG_WITHDATA;
		} else {
			return MSG_ONLY;
		}
	} else {
		return MSG_NONE;
	}
}

int HttpMsgFrame2::fetchData(string& data) {
	if(!mDataList.empty()) {
		assert(mMsgList.empty()==false || mDataList.front().seq < mMsgList.front().seq);
		string res;
		auto &dataseq = mDataList.front();
		res = move(dataseq.data);
		mDataList.pop_front();
		data = move(res);
		return MSG_DATA;
	} else {
		if(!mBodyData.empty()) {
			assert(mMsgList.empty()==true);
			data = move(mBodyData);
			mBodyData.clear();
			return MSG_DATA;
		}
		return MSG_DATA_EMPTY;
	}
}


size_t HttpMsgFrame2::feedPacket(const char* buf, size_t len) {
	auto ret = http_parser_execute(&mParser, &_parserSettings, buf, len);
//	ald("parse ret=%d", ret);
	return ret;
}

size_t HttpMsgFrame2::feedPacket(vector<char> && pkt) {
	return feedPacket(pkt.data(), pkt.size());
}

int HttpMsgFrame2::init(bool isreq) {
	clear();
	mIsReq = isreq;
	initHttpParser();
	return 0;
}

int HttpMsgFrame2::status() const {
	uint32_t dseq=UINT32_MAX;
	uint32_t mseq=UINT32_MAX;
	if(!mMsgList.empty()) {
		mseq = mMsgList.front().seq;
	}
	if(!mDataList.empty()) {
		dseq = mDataList.front().seq;
	} else {
		if(!mBodyData.empty()) {
			dseq = mMsgSeqNum;
		}
	}

	if(mseq != UINT32_MAX) {
		if(mseq <= dseq) {
			return FS_HDR;
		}
	}

	if(dseq != UINT32_MAX) {
		return FS_DATA;
	}

	return FS_NONE;
}

void HttpMsgFrame2::initHttpParser() {
	http_parser_init(&mParser, mIsReq ? HTTP_REQUEST : HTTP_RESPONSE);
	mParser.data = this;
}

int HttpMsgFrame2::head_field_cb(http_parser* parser, const char *at, size_t length) {
	HttpMsgFrame2* pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgHeaderNameCb(parser, at, length);
}

int HttpMsgFrame2::head_val_cb(http_parser* parser, const char *at, size_t length) {
	HttpMsgFrame2 *pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgHeaderValCb(parser, at, length);
}

int HttpMsgFrame2::body_cb(http_parser* parser, const char *at, size_t length) {
	HttpMsgFrame2 *pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgbodyDataCb(parser, at, length);
}

int HttpMsgFrame2::msg_begin(http_parser* parser) {
	HttpMsgFrame2 *pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgMsgBeginCb(parser);

}

int HttpMsgFrame2::msg_end(http_parser* parser) {
	HttpMsgFrame2 *pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgMsgEndCb(parser);
}

int HttpMsgFrame2::on_url(http_parser* parser, const char* at, size_t length) {
	HttpMsgFrame2 *pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgUrlCb(parser, at, length);
}

int HttpMsgFrame2::on_headers_complete(http_parser* parser) {
	HttpMsgFrame2 *pcnn = (HttpMsgFrame2*) parser->data;
	return pcnn->dgHeaderComp(parser);

}

int HttpMsgFrame2::on_status(http_parser* parser, const char* at, size_t length) {

	HttpMsgFrame2 *pf = (HttpMsgFrame2*) parser->data;
	return pf->dgFirstLineStatus(parser, at, length);
}

int HttpMsgFrame2::dgHeaderNameCb(http_parser*, const char* at, size_t length) {
	ald("parser hdr name cb, str=%s", string(at, length));
	if (mPs == PS_FIRST_LINE) {
		procFirstLine();
	}

	if (mCurHdrVal.empty() == false) {
		ald("header set, name=%s, val=%s", mCurHdrName, mCurHdrVal);
		procHeader();
	}

	mCurHdrName.append(at, length);
	return 0;
}

int HttpMsgFrame2::dgHeaderValCb(http_parser*, const char* at, size_t length) {
	ald("parser hdr val cb, str=%s", string(at, length));
	mCurHdrVal.append(at, length);
	return 0;

}

int HttpMsgFrame2::dgHeaderComp(http_parser* parser) {
	if (mCurHdrVal.empty() == false) {
		ald("header comp, name=%s, val=%s", mCurHdrName, mCurHdrVal);
		procHeader();
	}
//	mStatus = FS_HDR;
	mFrameStatus = FS_HDR;
	mContentLen = 0;
	if (parser->flags & F_CHUNKED) {
		mContentLen = -1;
	}
	else if (parser->content_length != ULLONG_MAX) {
		mContentLen = parser->content_length;
	}

	if (mIsReq) {
		mMsg->setMethod((http_method) parser->method);
	}
	else {
		mMsg->setRespStatus(parser->status_code);
	}

	mMsg->setContentLenInt(mContentLen);
	mMsg->setParserFlag(parser->flags);
	if (parser->protocol_type == 'R') {
		mMsg->setProtocolVer("RTSP/1.0");
	}
	ald("request header ok.");
	ald("    resp code: %d", mMsg->getRespStatus());
	ald("    content-len: %ld", parser->content_length);
	ald("    chunked: %d", parser->flags & F_CHUNKED);
	ald("    header dump: \n%s", mMsg->dumpHdr());
	ald("header end...");

	mReadStatus = 0x01;
	if (mContentLen != 0) {
		mReadStatus |= 0x02;
	}
//	if(!mIsReq && (parser->content_length == 0 || parser->content_length == ULLONG_MAX)) {
//		return 1;
//	}

	mMsgList.emplace_back();
	auto &mseq = mMsgList.back();
	mseq.seq = mMsgSeqNum;
	mseq.msg = move(mMsg);
	ald("stacked msg count=%d", mMsgList.size());
	return 0;
}

int HttpMsgFrame2::dgbodyDataCb(http_parser* parser, const char* at, size_t length) {
	ald("body data cb..., data=%s", string(at, length));
	if (mPs == PS_HEADER) {
		mPs = PS_BODY;
		mFrameStatus = FS_DATA;
	}

	mBodyData.append(at, length);

	return 0;
}

int HttpMsgFrame2::dgMsgBeginCb(http_parser* parser) {
	++mMsgSeqNum;
	if(mMsgSeqNum==UINT32_MAX) mMsgSeqNum = 0;

	ald("msg begin cb..., msg_seq_num=%d", mMsgSeqNum);
	mMsg.reset(new BaseMsg(mIsReq ? BaseMsg::REQUEST: BaseMsg::RESPONSE));
	mBodyData.clear();
	mPs = PS_FIRST_LINE;
	mReadLen = 0;
	mContentLen = 0;
	mReadStatus = 0;

	return 0;
}

int HttpMsgFrame2::dgMsgEndCb(http_parser* parser) {
	ald("msg end cb...");
	if(mBodyData.empty() == false) {
		mDataList.emplace_back();
		auto &ds = mDataList.back();
		ds.data = mBodyData; mBodyData.clear();
		ds.seq = mMsgSeqNum;
		ald("stacked data count=%d", mDataList.size());
	}

	// stack message end signal
	mDataList.emplace_back();
	auto &end = mDataList.back();
	end.seq = mMsgSeqNum;

//	mPs = PS_INIT;
	mPs = PS_END;
	return 0;
}

int HttpMsgFrame2::chunk_header_cb(http_parser* parser) {
	ald("chunk header cb...");
	return 0;
}

int HttpMsgFrame2::chunk_comp_cb(http_parser* parser) {
	ald("chunk complete cb...");
	return 0;
}

void HttpMsgFrame2::procFirstLine() {
	ald("proc first line");
	mPs = PS_HEADER;
}

void HttpMsgFrame2::procHeader() {
//	if (mCurCtrl != NULL)
//		mCurCtrl->addReqHeader(move(mCurHdrName), move(mCurHdrVal));

	mMsg->addHdr(mCurHdrName, mCurHdrVal);
	mCurHdrName.clear();
	mCurHdrVal.clear();
}

int HttpMsgFrame2::dgUrlCb(http_parser* parser, const char* at, size_t length) {
	ald("url cb");
	ald("  %s", string(at, length));
	mMsg->setUrl(at, length);
	return 0;
}

void HttpMsgFrame2::recycleDataBuf(string&& bufstr) {
	bufstr.clear();
	mBodyData = move(bufstr);
}

void HttpMsgFrame2::clear() {
	mPs = PS_INIT;
	mContentLen = 0;
	mReadLen = 0;
	mReadStatus = 0;
	if(mMsg) mMsg->clear();
	mBodyData.clear();
	mMsgList.clear();
}

string HttpMsgFrame2::pullPacket() {
	string ts = move(mEncPkt);
	mEncPkt.clear();
	return move(ts);
}

int HttpMsgFrame2::dgFirstLineStatus(http_parser* parser, const char* at, size_t length) {
	ald("first line status: code=%d", parser->status_code);
	ald("   status=%s", string(at, length));
	return 0;
}

void HttpMsgFrame2::setHost(const string& hostname) {
	mHost = hostname;
}

void HttpMsgFrame2::setUserAgent(const string& agent) {
	mAgent = agent;
}

void HttpMsgFrame2::frameMsg(BaseMsg& msg) {
	mEncPkt = msg.serialize();
	ald("frame msg: \n%s", mEncPkt);
}

void HttpMsgFrame2::frameData(string&& data) {
	if (mEncPkt.empty() == true) {
		ald("frame data move...");
		mEncPkt = move(data);
	}
	else {
		ald("frame data append...");
		mEncPkt.append(data);
	}
}

bool HttpMsgFrame2::isEmptyPacket() {
	return mEncPkt.empty();
}
}
