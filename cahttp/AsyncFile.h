/*
 * AsyncFile.h
 *
 *  Created on: Feb 4, 2016
 *      Author: netmind
 */

#ifndef ASYNCFILE_H_
#define ASYNCFILE_H_

#include <memory>
#include <functional>
#include <ednio/EdNio.h>

namespace cahttp {

class AsyncFile {
public:
	typedef std::function< void (std::unique_ptr<char[]>, size_t)> Lis;
	AsyncFile();
	virtual ~AsyncFile();
	int open(const std::string& path, Lis lis);
	void close();
	void recycleBuf(std::unique_ptr<char[]> buf, size_t len);
private:
	FILE *mSt;
	edft::EdEventFd mEvt;
	size_t mChkSize;
	size_t mDataSize;
	Lis mLis;
	std::unique_ptr<char[]> mBuf;
	size_t mBufLen;
};

} // namespace

#endif /* ASYNCFILE_H_ */
