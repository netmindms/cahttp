/*
 * TransfEncStrm.h
 *
 *  Created on: Jan 18, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_TRANSFENCSTRM_H_
#define CAHTTP_TRANSFENCSTRM_H_

#include <string>
#include "HttpBaseReadStream.h"

namespace cahttp {

class TransfEncStrm : public HttpBaseReadStream {
public:
	TransfEncStrm();
	virtual ~TransfEncStrm();
	virtual std::pair<const char*, int64_t> getDataPtr();
	virtual size_t remain();
	virtual void consume(size_t len);
	virtual ssize_t store(const char* ptr, size_t len);
	void setStream(HttpBaseReadStream* pstrm);
private:
	HttpBaseReadStream* mpOrgStrm;
	char* mBuf;
	size_t mBufSize;
	size_t mChunkSize;
};

} /* namespace cahttp */

#endif /* CAHTTP_TRANSFENCSTRM_H_ */
