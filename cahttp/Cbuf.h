/*
 * Cbuf.h
 *
 *  Created on: Feb 5, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_CBUF_H_
#define CAHTTP_CBUF_H_

#include <utility>
#include <memory>

namespace cahttp {

struct CBuf {
	CBuf() {
		len=0;
	}
	std::unique_ptr<char[]> buf;
	size_t len;
};

}

#endif /* CAHTTP_CBUF_H_ */
