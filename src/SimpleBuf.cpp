/*
 * SimpleBuf.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: netmind
 */

#include "SimpleBuf.h"

SimpleBuf::SimpleBuf() {
	buffer = nullptr;
	capacity = 0;
	size = 0;
}

SimpleBuf::~SimpleBuf() {
	if(buffer) {
		free(buffer);
	}
}

void SimpleBuf::alloc(size_t bsize) {
	if(buffer) {
		free(buffer);
	}
	buffer = (char*)malloc(bsize);
	if(buffer) {
		capacity = bsize;
	} else {
		capacity = 0;
	}

}
