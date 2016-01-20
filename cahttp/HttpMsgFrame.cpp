/*
 * HttpMsgFrame.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG
#include <climits>
#include "flog.h"
#include "HttpMsgFrame.h"

namespace cahttp {
static http_parser_settings _parserSettings;

class _HTTP_MSG_FRAME_MODULE_INITIALIZER {
public:
	_HTTP_MSG_FRAME_MODULE_INITIALIZER() {
		_parserSettings.on_message_begin = HttpMsgFrame::msg_begin;
		_parserSettings.on_message_complete = HttpMsgFrame::msg_end;
		_parserSettings.on_url = HttpMsgFrame::on_url;
		_parserSettings.on_status = HttpMsgFrame::on_status;
		_parserSettings.on_header_field = HttpMsgFrame::head_field_cb;
		_parserSettings.on_header_value = HttpMsgFrame::head_val_cb;
		_parserSettings.on_headers_complete = HttpMsgFrame::on_headers_complete;
		_parserSettings.on_body = HttpMsgFrame::body_cb;
		_parserSettings.on_chunk_header = HttpMsgFrame::chunk_header_cb;
		_parserSettings.on_chunk_complete = HttpMsgFrame::chunk_comp_cb;
	}

};

static _HTTP_MSG_FRAME_MODULE_INITIALIZER _mod_init;

HttpMsgFrame::HttpMsgFrame() {
	mIsServer = false;
//	mStatus = 0;
	mPs = PS_INIT;
	mFrameStatus = FS_IDLE;
	mReadLen = 0;
	mReadStatus = 0;
	mContentLen = 0;
}

HttpMsgFrame::~HttpMsgFrame() {
}

#if 1
int HttpMsgFrame::fetchMsg(CaHttpMsg& msg) {
	if (mMsgList.size() == 0) {
		if (mReadStatus & 0x01) {
			msg = move(mMsg);
			mReadStatus &= 0xfe;
			if (mContentLen == 0) {
				return MSG_ONLY;
			}
			else
				return MSG_WITHDATA;
		}
		return FETCH_ERROR;

	}
	else {
		ald("msg que size=%d", mMsgList.size());
		auto &frame = mMsgList.front();
		if (frame.readstatus & 0x01) {
			frame.readstatus &= 0xfe;
			msg = move(frame.msg);
			if (msg.getContentLenInt() == 0) {
				mMsgList.pop_front();
				return MSG_ONLY;
			}
			else {
				assert(msg.getContentLenInt() != -1);
				return MSG_WITHDATA;
			}
		}
		return FETCH_ERROR;
	}
}
#else
CaHttpMsg HttpMsgFrame::fetchMsg() {
#if 0
	auto itr = mMsgList.begin();
	if(itr != mMsgList.end()) {
		CaHttpMsg *msg = (CaHttpMsg*)itr->second;
		mMsgList.erase(itr);
		return move(*msg);
	}
	else {
		assert(0);
		return CaHttpMsg();
	}
#else
	return move(mMsg);
#endif
}
#endif

#if 1
int HttpMsgFrame::fetchData(string& data) {
	ald("fetchData, read status=%d, cur data len=%ld", mReadStatus, mBodyData.size());
	if (mMsgList.size() == 0) {
		assert(mContentLen != 0);
		if(mBodyData.size()>0) {
			mReadLen += mBodyData.size();
			data = move(mBodyData);
			mBodyData.clear();
			if (mContentLen > 0) {
				if (mReadLen >= mContentLen) {
					assert(mReadLen == mContentLen);
					mReadStatus &= 0xfd;
					return MSG_DATA_END;
				}
				else {
					return MSG_DATA;
				}
			}
			else {
				return MSG_DATA;
			}
		} else {
			data.clear();
			if(mContentLen<0 && mPs == PS_END) {
				return MSG_DATA_END;
			}
			return MSG_DATA_EMPTY;
		}
	} else {
		auto &frame = mMsgList.front();
		if (frame.msg.getMsgClass() == 0) {
			data = move(frame.data);
			frame.data.clear();
			mMsgList.pop_front();
			return MSG_DATA_END;
		}
		else {
			assert(0);
			return FETCH_ERROR;
		}
	}
}
#else
string HttpMsgFrame::fetchData() {
#if 1
	auto itr = mMsgList.begin();
	if(itr != mMsgList.end()) {
		string *ps = (string*)itr->second;
	}
	else {
		assert(0);
		return "";
	}
#else
	auto ds = move(mBodyData);
	mBodyData.clear();
	return move(ds);
#endif
}
#endif

size_t HttpMsgFrame::feedPacket(const char* buf, size_t len) {
	auto ret = http_parser_execute(&mParser, &_parserSettings, buf, len);
//	ald("parse ret=%d", ret);
	return ret;
}

size_t HttpMsgFrame::feedPacket(vector<char> && pkt) {
	return feedPacket(pkt.data(), pkt.size());
}

int HttpMsgFrame::init(bool server) {
	clear();
	mIsServer = server;
	initHttpParser();
	return 0;
}

int HttpMsgFrame::status() const {
	uint8_t rs;
	if (mMsgList.size() == 0) {
		rs = mReadStatus;
	}
	else {
		rs = mMsgList.front().readstatus;
	}
	if (rs & 0x01) {
		return FS_HDR;
	}
	else if (rs & 0x02) {
		return FS_DATA;
	}
	else
		return FS_NONE;
}

void HttpMsgFrame::initHttpParser() {
	http_parser_init(&mParser, mIsServer ? HTTP_REQUEST : HTTP_RESPONSE);
	mParser.data = this;
}

int HttpMsgFrame::head_field_cb(http_parser* parser, const char *at, size_t length) {
	HttpMsgFrame* pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgHeaderNameCb(parser, at, length);
}

int HttpMsgFrame::head_val_cb(http_parser* parser, const char *at, size_t length) {
	HttpMsgFrame *pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgHeaderValCb(parser, at, length);
}

int HttpMsgFrame::body_cb(http_parser* parser, const char *at, size_t length) {
	HttpMsgFrame *pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgbodyDataCb(parser, at, length);
}

int HttpMsgFrame::msg_begin(http_parser* parser) {
	HttpMsgFrame *pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgMsgBeginCb(parser);

}

int HttpMsgFrame::msg_end(http_parser* parser) {
	HttpMsgFrame *pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgMsgEndCb(parser);
}

int HttpMsgFrame::on_url(http_parser* parser, const char* at, size_t length) {
	HttpMsgFrame *pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgUrlCb(parser, at, length);
}

int HttpMsgFrame::on_headers_complete(http_parser* parser) {
	HttpMsgFrame *pcnn = (HttpMsgFrame*) parser->data;
	return pcnn->dgHeaderComp(parser);

}

int HttpMsgFrame::on_status(http_parser* parser, const char* at, size_t length) {

	HttpMsgFrame *pf = (HttpMsgFrame*) parser->data;
	return pf->dgFirstLineStatus(parser, at, length);
}

int HttpMsgFrame::dgHeaderNameCb(http_parser*, const char* at, size_t length) {
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

int HttpMsgFrame::dgHeaderValCb(http_parser*, const char* at, size_t length) {
	ald("parser hdr val cb, str=%s", string(at, length));
	mCurHdrVal.append(at, length);
	return 0;

}

int HttpMsgFrame::dgHeaderComp(http_parser* parser) {
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

	if (mIsServer) {
		mMsg.setMethod((http_method) parser->method);
	}
	else {
		mMsg.setResponse(parser->status_code);
	}

	mMsg.setContentLenInt(mContentLen);
	mMsg.setMsgFlag(parser->flags);
	if (parser->protocol_type == 'R') {
		mMsg.setProtocolVer("RTSP/1.0");
	}
	ald("request header ok.");
	ald("    resp code: %d", mMsg.getRespCode());
	ald("    content-len: %ld", parser->content_length);
	ald("    chunked: %d", parser->flags & F_CHUNKED);
	ald("    header dump: \n%s", mMsg.dumpHdr());
	ald("header end...");

	mReadStatus = 0x01;
	if (mContentLen != 0) {
		mReadStatus |= 0x02;
	}
//	if(!mIsServer && (parser->content_length == 0 || parser->content_length == ULLONG_MAX)) {
//		return 1;
//	}
	return 0;
}

int HttpMsgFrame::dgbodyDataCb(http_parser* parser, const char* at, size_t length) {
	ald("body data cb..., data=%s", string(at, length));
	if (mPs == PS_HEADER) {
		mPs = PS_BODY;
		mFrameStatus = FS_DATA;
	}

	mBodyData.append(at, length);

	return 0;
}

int HttpMsgFrame::dgMsgBeginCb(http_parser* parser) {
	ald("msg begin cb...");
	if (mMsg.getMsgClass()) {
		mMsgList.emplace_back();
		auto &frame = mMsgList.back();
		frame.msg = move(mMsg);
		frame.readstatus = 0x01;
		if (mBodyData.empty() == false) {
			frame.data = move(mBodyData);
			frame.readstatus |= 0x02;
		}
		mBodyData.clear();
	}
	mPs = PS_FIRST_LINE;
	mReadLen = 0;
	mContentLen = 0;
	mReadStatus = 0;

	return 0;
}

int HttpMsgFrame::dgMsgEndCb(http_parser* parser) {
	ald("msg end cb...");
//	mPs = PS_INIT;
	mPs = PS_END;
	return 0;
}

int HttpMsgFrame::chunk_header_cb(http_parser* parser) {
	ald("chunk header cb...");
	return 0;
}

int HttpMsgFrame::chunk_comp_cb(http_parser* parser) {
	ald("chunk complete cb...");
	return 0;
}

void HttpMsgFrame::procFirstLine() {
	ald("proc first line");
	mPs = PS_HEADER;
}

void HttpMsgFrame::procHeader() {
//	if (mCurCtrl != NULL)
//		mCurCtrl->addReqHeader(move(mCurHdrName), move(mCurHdrVal));

	mMsg.setHdr(move(mCurHdrName), move(mCurHdrVal));
	mCurHdrName.clear();
	mCurHdrVal.clear();
}

int HttpMsgFrame::dgUrlCb(http_parser* parser, const char* at, size_t length) {
	ald("url cb");
	ald("  %s", string(at, length));
	mMsg.setUrl(at, length);
	return 0;
}

void HttpMsgFrame::recycleDataBuf(string&& bufstr) {
	bufstr.clear();
	mBodyData = move(bufstr);
}

void HttpMsgFrame::clear() {
	mPs = PS_INIT;
	mContentLen = 0;
	mReadLen = 0;
	mReadStatus = 0;
	mMsg.clear();
	mBodyData.clear();
	mMsgList.clear();
}

string HttpMsgFrame::pullPacket() {
	string ts = move(mEncPkt);
	mEncPkt.clear();
	return move(ts);
}

int HttpMsgFrame::dgFirstLineStatus(http_parser* parser, const char* at, size_t length) {
	ald("first line status: code=%d", parser->status_code);
	ald("   status=%s", string(at, length));
	return 0;
}

void HttpMsgFrame::setHost(const string& hostname) {
	mHost = hostname;
}

void HttpMsgFrame::setUserAgent(const string& agent) {
	mAgent = agent;
}

void HttpMsgFrame::frameMsg(CaHttpMsg& msg) {
	mEncPkt = msg.serialize();
	ald("frame msg: \n%s", mEncPkt);
}

void HttpMsgFrame::frameData(string&& data) {
	if (mEncPkt.empty() == true) {
		ald("frame data move...");
		mEncPkt = move(data);
	}
	else {
		ald("frame data append...");
		mEncPkt.append(data);
	}
}

bool HttpMsgFrame::isEmptyPacket() {
	return mEncPkt.empty();
}
}
