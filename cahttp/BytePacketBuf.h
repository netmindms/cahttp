/*
 * BytePacketBuf.h
 *
 *  Created on: Feb 2, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_BYTEPACKETBUF_H_
#define CAHTTP_BYTEPACKETBUF_H_
#include "PacketBuf.h"

namespace cahttp {

class BytePacketBuf: public PacketBuf {
public:
	BytePacketBuf();
	virtual ~BytePacketBuf();
	virtual size_t remain() override;
	virtual void consume() override;
	virtual std::pair<size_t, const char*> getBuf() override;
	void allocBuf(size_t size);
	void setData(const char* ptr, size_t len);
	void addData(const char* ptr, size_t len);
private:
	char* mBuf;
	size_t mBufSize;
	size_t mDataSize;
};

} /* namespace cahttp */

#endif /* CAHTTP_BYTEPACKETBUF_H_ */
