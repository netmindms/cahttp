/*
 * MultiStream.h
 *
 *  Created on: Jan 17, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_MULTISTREAM_H_
#define CAHTTP_MULTISTREAM_H_

#include <list>
#include "HttpBaseReadStream.h"

namespace cahttp {

class MultiStream {
public:
	MultiStream();
	virtual ~MultiStream();
	void addStrm(HttpBaseReadStream* pstrm);
	std::pair<const char*, size_t> getDataPtr();
	void consume(size_t len);
private:
	std::list<HttpBaseReadStream*> mStrmList;
};

} /* namespace cahttp */

#endif /* CAHTTP_MULTISTREAM_H_ */
