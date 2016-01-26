/*
 * HttpMsgFrame2.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */
#define LOG_LEVEL LOG_DEBUG
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
	mIsServer = false;
//	mStatus = 0;
	mPs = PS_INIT;
	mFrameStatus = FS_IDLE;
	mReadLen = 0;
	mReadStatus = 0;
	mContentLen = 0;
}

HttpMsgFrame2::~HttpMsgFrame2() {
}

int HttpMsgFrame2::fetchMsg(CaHttpMsg& msg) {
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

int HttpMsgFrame2::fetchData(string& data) {
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


size_t HttpMsgFrame2::feedPacket(const char* buf, size_t len) {
	auto ret = http_parser_execute(&mParser, &_parserSettings, buf, len);
//	ald("parse ret=%d", ret);
	return ret;
}

size_t HttpMsgFrame2::feedPacket(vector<char> && pkt) {
	return feedPacket(pkt.data(), pkt.size());
}

int HttpMsgFrame2::init(bool server) {
	clear();
	mIsServer = server;
	initHttpParser();
	return 0;
}

int HttpMsgFrame2::status() const {
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

void HttpMsgFrame2::initHttpParser() {
	http_parser_init(&mParser, mIsServer ? HTTP_REQUEST : HTTP_RESPONSE);
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

int HttpMsgFrame2::dgMsgEndCb(http_parser* parser) {
	ald("msg end cb...");
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

	mMsg.setHdr(move(mCurHdrName), move(mCurHdrVal));
	mCurHdrName.clear();
	mCurHdrVal.clear();
}

int HttpMsgFrame2::dgUrlCb(http_parser* parser, const char* at, size_t length) {
	ald("url cb");
	ald("  %s", string(at, length));
	mMsg.setUrl(at, length);
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
	mMsg.clear();
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

void HttpMsgFrame2::frameMsg(CaHttpMsg& msg) {
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
