/*
 * PacketBuf.h
 *
 *  Created on: Jan 31, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_PACKETBUF_H_
#define CAHTTP_PACKETBUF_H_

#include <utility>
#include <unistd.h>

namespace cahttp {

class PacketBuf {
public:
	PacketBuf();
	virtual ~PacketBuf();
	virtual size_t remain()=0;
	virtual void consume()=0;
	virtual std::pair<size_t, const char*> getBuf()=0;
};

} /* namespace cahttp */

#endif /* CAHTTP_PACKETBUF_H_ */
