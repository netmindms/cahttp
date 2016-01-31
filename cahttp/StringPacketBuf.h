/*
 * StringPacketBuf.h
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_STRINGPACKETBUF_H_
#define CAHTTP_STRINGPACKETBUF_H_

#include <string>
#include "PacketBuf.h"

namespace cahttp {

class StringPacketBuf: public PacketBuf {
public:
	StringPacketBuf();
	StringPacketBuf(const std::string &s);
	virtual ~StringPacketBuf();
	void setString(std::string &&s);
	void setString(const char* ptr, size_t len);

private:
	std::string mString;

	void consume() override;
	size_t remain() override;
	std::pair<size_t, const char*> getBuf() override;
};

} /* namespace cahttp */

#endif /* CAHTTP_STRINGPACKETBUF_H_ */
