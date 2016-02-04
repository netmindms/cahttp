/*
 * etcutil.h
 *
 *  Created on: Aug 22, 2015
 *      Author: root
 */

#ifndef EXTERNAL_UTIL_ETCUTIL_H_
#define EXTERNAL_UTIL_ETCUTIL_H_

#include <chrono>

namespace cahttpu {

#define NOW() std::chrono::system_clock::now()
#define DURMSEC(t1, t2) std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
typedef std::chrono::system_clock::time_point sctime_t;

}

#endif /* EXTERNAL_UTIL_ETCUTIL_H_ */
