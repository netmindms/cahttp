/*
 * util.h
 *
 *  Created on: Feb 1, 2016
 *      Author: netmind
 */

#ifndef CAHTTP_UTIL_H_
#define CAHTTP_UTIL_H_

#include <chrono>

#define NOW() std::chrono::system_clock::now()
#define DURMSEC(t1, t2) std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
typedef std::chrono::system_clock::time_point sctime_t;

// flag must be unsigned
#define BIT_SET(flag, POS) { flag |= (1<<POS); }
#define BIT_RESET(flag, POS) { flag &= (~(1<<POS)); }
#define BIT_TEST(flag, POS)  (flag & (1<<POS) )
#define BIT_FRAG(flag, POS_H, POS_L)  ( ((decltype(flag))(flag<<(sizeof(flag)*8-POS_H-1))) >>(sizeof(flag)*8-(POS_H-P


#endif /* CAHTTP_UTIL_H_ */
