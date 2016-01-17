/*
 * SDPParser.cpp
 *
 *  Created on: Oct 9, 2015
 *      Author: netmind
 */

#include <string>
#include "SDPParser.h"

using namespace std;
/* sample
 v=0
 o=AirTunes 7138256552664387265 0 IN IP4 192.168.0.7
 s=AirTunes
 i=GyungMo Kang... iPad
 c=IN IP4 192.168.0.7
 t=0 0
 m=audio 0 RTP/AVP 96
 a=rtpmap:96 mpeg4-generic/44100/2
 a=fmtp:96 mode=AAC-eld; constantDuration=480
 a=fpaeskey:RlBMWQECAQAAAAA8AAAAAMnrW+wveZewgudfPasKwjYAAAAQmkznytsUzkYZijwo1F3y2lFGU6HMWdFTTFJxLVNwhXj+Qjzu
 a=aesiv:NZssznYsdn+T2Md+Zcg7fQ==
 a=min-latency:3750
 a=max-latency:3750



 */

enum state_e {
	s_sc,
	s_eq,
	s_consume,
	s_attr_name,
	s_attr_val,
	s_ver,
	s_media,
	s_port,
	s_port_num,
	s_proto,
	s_sp,
	s_nettype,
	s_addrtype,
	s_addr,
	s_format,
	s_addr_end_sp,
	s_slash,
	s_ttl,
	s_start_time,
	s_end_time,
	s_endline,
	s_linestart,
	s_user_name,
	s_sess_id,
	s_sess_ver,
};

SDPParser::SDPParser() {
	mError = NONE;
	startTime = endTime = 0;
	mVer = 0;
	ttl = 0;
}

SDPParser::~SDPParser() {
}

size_t SDPParser::parse(const char* data, size_t len) {
	char ch;
	const char* ptr = data;
	int rcnt, remain;
	media_t *pmedia=nullptr;
	attr_t *pattr=nullptr;
	for (remain = len; remain > 0;) {
		ch = *ptr;
		switch (ch) {
		case 'v':
			rcnt = parse_v(ptr, remain);
			break;
		case 's':
			rcnt = parse_simple_line(ptr, remain, 's', sessionName);
			break;
		case 'm':
			mMedias.emplace_back();
			pmedia = &(mMedias.back());
			rcnt = parse_m(ptr, remain, *pmedia);
			break;
		case 'a':
			if(pmedia) {
				pmedia->attrs.emplace_back();
				pattr = &(pmedia->attrs.back());
			} else {
				mAttrs.emplace_back();
				pattr = &(mAttrs.back());
			}
			rcnt = parse_a(ptr, remain, *pattr);
			break;
		case 'o':
			rcnt = parse_o(ptr, remain);
			break;
		case 'c':
			rcnt = parse_c(ptr, remain);
			break;
		case 'i':
			rcnt = parse_simple_line(ptr, remain, 'i', mSessInfo);
			break;
		case 't':
			rcnt = parse_t(ptr, remain);
			break;
		default:
			{
				string dummy;
				rcnt = parse_simple_line(ptr, remain, ch, dummy);
			}
			break;

		}
		if (rcnt < 0)
			return -1;
		remain -= rcnt;
		ptr += rcnt;
	}
	return 0;
}

vector<SDPParser::media_t>& SDPParser::getAllMedias() {
	return mMedias;
}

SDPParser::media_t* SDPParser::getMedia(const char* media) {
	for(auto &m: mMedias) {
		if(m.media == media) {
			return &m;
		}
	}
	return nullptr;
}

const string& SDPParser::getMediaAttr(const char* media, const char* attr_name,	const string& def) {
	media_t* pm=nullptr;
	for(auto &m: mMedias) {
		if(m.media == media) {
			pm = &m;
		}
	}
	if(pm) {
		for(auto &attr: pm->attrs) {
			if(attr.first == attr_name) {
				return attr.second;
			}
		}
		return def;
	}
	else {
		return def;
	}
}

SDPParser::attr_t* SDPParser::getAttr(media_t& media, const char* attr_name) {
	for(auto &a: media.attrs) {
		if(a.first == attr_name) {
			return &a;
		}
	}
	return nullptr;
}

vector<SDPParser::attr_t>& SDPParser::getAllAttr(media_t& media) {
	return media.attrs;
}

size_t SDPParser::parse_simple_line(const char* ptr, size_t len, char sc,
		string& line) {
	size_t remain;
	char ch;
	state_e stt, next_stt;
	stt = s_sc;
	line.clear();
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			stt = s_endline;
		} else if(stt == s_linestart) {
			line.push_back(ch);
		}
		else if (stt == s_sp) {
			if (ch != ' ') {
				stt = next_stt;
				ptr--;
				remain++;
			}
		} else if (stt == s_sc) {
			if (ch == sc) {
				stt = s_eq;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_eq) {
			if (ch == '=') {
				stt = s_linestart;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_endline) {
			remain++;
			break;
		} else if (stt == s_consume) {

		} else {
			mError = UNKNOWN;
			remain = len;
			break;
		}

	}
	return (len - remain);
}

size_t SDPParser::parse_v(const char* ptr, size_t len) {

	state_e stt = s_sc;
	int remain;
	char ch;
	mVer = 0;
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			stt = s_endline;
		} else if (stt == s_sc) {
			if (ch == 'v') {
				stt = s_eq;
			}
		} else if (stt == s_eq) {
			if (ch == '=') {
				stt = s_ver;
			} else {
				return -1;
			}
		} else if (stt == s_ver) {
			if (ch >= '0' && ch <= '9') {
				mVer = mVer * 10 + ch - '0';
			} else {
				// just consume
			}

		} else if (stt == s_endline) {
			remain++;
			break;
		}

	}
	return (len - remain);
}

size_t SDPParser::parse_o(const char* ptr, size_t len) {
	 //o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
	origin.sessVer=0;
	size_t remain;
	char ch;
	state_e stt, next_stt;
	stt = s_sc;
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			stt = s_endline;
		} else if(stt == s_user_name) {
			if(ch != ' ') {
				origin.userName.push_back(ch);
			}
			else {
				stt = s_sp;next_stt = s_sess_id;
			}
		} else if(stt == s_sess_id) {
			if(ch != ' ') {
				origin.sessId.push_back(ch);
			}
			else {
				stt = s_sp; next_stt = s_sess_ver;
			}
		} else if(stt == s_sess_ver) {
			if(ch >= '0' && ch <= '9') {
				origin.sessVer = origin.sessVer*10 + ch - '0';
			} else if(ch == ' ') {
				stt = s_sp; next_stt = s_nettype;
			} else {
				mError = O_ERROR;
				stt = s_consume;
			}
		} else if(stt == s_nettype) {
			if(ch != ' ') {
				origin.nettype.push_back(ch);
			} else {
				stt = s_sp; next_stt = s_addrtype;
			}
		} else if(stt == s_addrtype) {
			if(ch != ' ') {
				origin.addrtype.push_back(ch);
			} else {
				stt = s_sp; next_stt=s_addr;
			}
		} else if(stt == s_addr) {
			if(ch != ' ') {
				origin.addr.push_back(ch);
			} else {
				stt = s_sp; next_stt=s_consume;
			}
		}

		else if (stt == s_sp) {
			if (ch != ' ') {
				stt = next_stt;
				ptr--;
				remain++;
			}
		} else if (stt == s_sc) {
			if (ch == 'o') {
				stt = s_eq;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_eq) {
			if (ch == '=') {
				stt = s_user_name;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_endline) {
			remain++;
			break;
		} else if (stt == s_consume) {

		} else {
			mError = UNKNOWN;
			remain = len;
			break;
		}

	}
	return (len - remain);

}

size_t SDPParser::parse_m(const char* ptr, size_t len, media_t &media) {
	//format-> m=<media> <port>/<number of ports> <proto> <fmt> ...

	size_t remain;
	state_e next_stt;
	state_e stt = s_sc;
	char ch;
	int format;
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			if (stt == s_format) {
				media.formats.push_back(format);
				format = 0;
			}
			stt = s_endline;
		} else if (stt == s_sc) {
			if (ch == 'm') {
				stt = s_eq;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_sp) {
			if (ch != ' ') {
				ptr--;
				remain++;
				stt = next_stt;
			}
		} else if (stt == s_eq) {
			if (ch == '=') {
				stt = s_media;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_media) {
			if (ch != ' ') {
				media.media.push_back(ch);
			} else {
				stt = s_sp;
				next_stt = s_port;
				media.port = 0;
				media.port_num = 0;
			}
		} else if (stt == s_port) {
			if (ch >= '0' && ch <= '9') {
				media.port = media.port * 10 + ch - '0';
			} else if (ch == '/') {
				media.port_num = 0;
				stt = s_port_num;
			} else if (ch == ' ') {
				stt = s_sp;
				next_stt = s_proto;
			}
		} else if (stt == s_port_num) {
			if (ch >= '0' && ch <= '9') {
				media.port_num = media.port_num * 10 + ch - '0';
			} else if (ch == ' ') {
				stt = s_sp;
				next_stt = s_proto;
			}
		} else if (stt == s_proto) {
			if (ch != ' ') {
				media.proto.push_back(ch);
			} else {
				stt = s_sp;
				next_stt = s_format;
				format = 0;
			}
		} else if (stt == s_format) {
			if (ch != ' ') {
				format = format * 10 + ch - '0';
			} else if (ch == ' ') {
				media.formats.push_back(format);
				stt = s_sp;
				next_stt = s_format;
				format = 0;
			}
		} else if (stt == s_consume) {

		} else if (stt == s_endline) {
			remain++;
			break;
		}
	}
	return (len - remain);
}

size_t SDPParser::parse_a(const char* ptr, size_t len, attr_t &attr) {
	size_t remain;
	char ch;
	attr.first.clear();
	attr.second.clear();
	state_e stt;
	stt = s_sc;
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			stt = s_endline;
		} else if (stt == s_sc) {
			if (ch == 'a') {
				stt = s_eq;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_eq) {
			if (ch == '=') {
				stt = s_attr_name;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_attr_name) {
			if (ch == ':') {
				stt = s_attr_val;
			} else {
				attr.first.push_back(ch);
			}
		} else if (stt == s_attr_val) {
			attr.second.push_back(ch);
		} else if (stt == s_endline) {
			remain++;
			break;
		} else if (stt == s_consume) {

		}

	}
	return (len - remain);
}

size_t SDPParser::parse_c(const char* ptr, size_t len) {
	size_t remain;
	char ch;
	state_e stt = s_sc;
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			if (stt == s_ttl) {
				ttl = stoi(bufs);
			}
			stt = s_endline;
		} else if (stt == s_sc) {
			if (ch == 'c') {
				stt = s_eq;
			} else {
				remain = len;
				break;
			}
		} else if (stt == s_eq) {
			if (ch == '=')
				stt = s_nettype;
			else
				return -1;
		} else if (stt == s_nettype) {
			if (ch != ' ') {
				nettype.push_back(ch);
			} else {
				stt = s_addrtype;
			}
		} else if (stt == s_addrtype) {
			if (ch != ' ') {
				addrtype.push_back(ch);
			} else {
				stt = s_addr;
			}
		} else if (stt == s_addr) {
			if (ch == '/') {
				stt = s_slash;
			} else if (ch == ' ') {
				stt = s_addr_end_sp;
			} else {
				ipstr.push_back(ch);
			}
		} else if (stt == s_slash) {
			if (ch >= '0' && ch <= '9') {
				bufs.push_back(ch);
			}
		} else if (stt == s_addr_end_sp) {
			if (ch != ' ')
				return -1;
		} else if (stt == s_endline) {
			remain++;
			break;
		}
	}

	return (len - remain);
}

size_t SDPParser::parse_t(const char* ptr, size_t len) {
	size_t remain;
	char ch;
	state_e stt, next_stt;
	stt = s_sc;
	startTime = endTime = 0;
	for (remain = len; remain > 0;) {
		ch = *ptr++;
		remain--;
		if (ch == '\r' || ch == '\n') {
			if (stt != s_end_time) {
				mError = T_ERROR;
			}
			stt = s_endline;
		} else if (stt == s_sc) {
			if (ch == 't') {
				stt = s_eq;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_eq) {
			if (ch == '=') {
				stt = s_start_time;
				startTime = 0;
			} else {
				stt = s_consume;
			}
		} else if (stt == s_start_time) {
			if (ch >= '0' && ch <= '9') {
				startTime = startTime * 10 + ch - '0';
			} else if (ch == ' ') {
				stt = s_end_time;
			} else {
				mError = T_ERROR;
				stt = s_consume;
			}
		} else if (stt == s_end_time) {
			if (ch >= '0' && ch <= '9') {
				endTime = endTime * 10 + ch - '0';
			} else if (ch == ' ') {
				stt = s_sp;
				next_stt = s_consume;
			}
		} else if (stt == s_sp) {
			if (ch != ' ') {
				stt = next_stt;
				ptr--;
				remain++;
			}
		} else if (stt == s_endline) {
			remain++;
			break;
		} else if (stt == s_consume) {

		} else {
			mError = UNKNOWN;
			remain = len;
			break;
		}

	}
	return (len - remain);
}

size_t SDPParser::parse(const string& data) {
	return parse(data.data(), data.size());
}
