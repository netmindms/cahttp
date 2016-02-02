/*
 * FilePacketBuf.h
 *
 *  Created on: Feb 1, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_FILEPACKETBUF_H_
#define CAHTTP_FILEPACKETBUF_H_

#include <stdio.h>
#include <string>
#include "PacketBuf.h"

namespace cahttp {

class FilePacketBuf: public PacketBuf {
public:
	FilePacketBuf();
	virtual ~FilePacketBuf();
	virtual size_t remain() override;
	virtual void consume() override;
	virtual std::pair<size_t, const char*> getBuf() override;

	int open(std::string& path);
	void close();

private:
	FILE* mSt;
	char* mBuf;
	size_t mDataCnt;
	size_t mBufSize;
	int64_t mFileSize;
};

} /* namespace cahttp */

#endif /* CAHTTP_FILEPACKETBUF_H_ */
