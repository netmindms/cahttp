/*
 * HandleFactory.h
 *
 *  Created on: Jul 19, 2015
 *      Author: netmind
 */

#ifndef SRC_HANDLEFACTORY_H_
#define SRC_HANDLEFACTORY_H_

#include <mutex>

class HandleFactory {
public:
	HandleFactory();
	virtual ~HandleFactory();
	uint32_t newHandle();
private:
	uint32_t mSeed;
	std::mutex mMutex;
};

#endif /* SRC_HANDLEFACTORY_H_ */
