/*
 * SimpleHandle.h
 *
 *  Created on: Nov 13, 2015
 *      Author: netmind
 */

#ifndef SRC_SIMPLEHANDLE_H_
#define SRC_SIMPLEHANDLE_H_

#include <thread>
#include <mutex>

class SimpleHandle {
public:
	SimpleHandle();
	virtual ~SimpleHandle();
	uint32_t newHandle();
private:
	std::mutex mLock;
	uint32_t mSeed;
};

#endif /* SRC_SIMPLEHANDLE_H_ */
