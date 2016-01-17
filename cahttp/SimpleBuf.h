/*
 * SimpleBuf.h
 *
 *  Created on: Oct 21, 2015
 *      Author: netmind
 */

#ifndef SRC_SIMPLEBUF_H_
#define SRC_SIMPLEBUF_H_
#include <unistd.h>
#include <stdlib.h>
#include <memory>

class SimpleBuf {
public:
	SimpleBuf();
	SimpleBuf(SimpleBuf &&other) {
		*this = std::move(other);
	}
	virtual ~SimpleBuf();
	SimpleBuf& operator=(SimpleBuf&& other) {
		if(this != &other) {
			buffer = other.buffer; other.buffer = nullptr;
			capacity = other.capacity; other.capacity = 0;
			size = other.size; other.size = 0;
		}
		return *this;
	}
	void alloc(size_t bsize);

	char *buffer;
	size_t capacity;
	size_t size;
};

#endif /* SRC_SIMPLEBUF_H_ */
