/*
 * CBuffer.h
 *
 *  Created on: Sep 10, 2015
 *      Author: netmind
 */

#ifndef NMDUTIL_CBUFFER_H_
#define NMDUTIL_CBUFFER_H_

#include <stdint.h>
#include <cstddef>
#include <memory>

namespace cahttpu {
using namespace std;

class CBuffer {
public:
	CBuffer();
	CBuffer(CBuffer &&other);
	CBuffer& operator=(CBuffer &&other);

	virtual ~CBuffer();
	size_t size();
	size_t capacity();
	void append(const char *ptr, size_t len);
	void assign(unique_ptr<char> ptr, size_t len);
	const char* get();
	char* release();

private:
	char *mBuf;
	size_t mCapacity;
	size_t mSize;
};

} /* namespace nmdu */

#endif /* NMDUTIL_CBUFFER_H_ */
