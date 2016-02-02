/*
 * TEEndPacketBuf.h
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_CHUNKENDPACKETBUF_H_
#define CAHTTP_CHUNKENDPACKETBUF_H_

#include "PacketBuf.h"
namespace cahttp {

class TEEndPacketBuf: public PacketBuf {
public:
	TEEndPacketBuf();
	virtual ~TEEndPacketBuf();
	virtual size_t remain() override;
	virtual void consume() override;
	virtual std::pair<size_t, const char*> getBuf() override;

private:
	char mLineEnd[5];
	size_t mLen;
};

} /* namespace cahttp */

#endif /* CAHTTP_CHUNKENDPACKETBUF_H_ */
