/*
 * HttpCnnMan.h
 *
 *  Created on: Mar 12, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_HTTPCNNMAN_H_
#define CAHTTP_HTTPCNNMAN_H_

#include <memory>

#include "SharedCnn.h"
namespace cahttp {

class HttpCnnMan {
public:
	union cfg_t {
		uint8_t val;
		struct {
			uint8_t pipelining: 1; // req finished
		};
	};
	HttpCnnMan();
	virtual ~HttpCnnMan();
	std::pair<std::unique_ptr<SharedCnn>, int> connect(uint32_t ip, uint16_t port);
	size_t getPoolSize() {
		return mCnnPool.size();
	}
	void pipelining(bool pipe) {
		mCfg.pipelining = pipe;
	};
	void close();

	// idx: connection index in list
	std::pair<size_t, size_t> getChannelCount(size_t idx);

private:
	cfg_t mCfg;
	std::list<std::shared_ptr<BaseConnection>> mCnnPool;
	uint32_t mHandleSeed;
};

} /* namespace cahttp */

#endif /* CAHTTP_HTTPCNNMAN_H_ */
