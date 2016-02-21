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

extern int get_num_fds();

// flag must be unsigned
#define BIT_SET(flag, POS) flag |= (1<<POS)
#define BIT_RESET(flag, POS) flag &= (~(1<<POS))
#define BIT_TEST(flag, POS)  ( (flag>>POS) & 1)
#define BIT_FRAG(flag, POS_H, POS_L)  ( ((decltype(flag))(flag<<(sizeof(flag)*8-POS_H-1))) >>(sizeof(flag)*8-(POS_H-POS_L+1)))

}

#endif /* EXTERNAL_UTIL_ETCUTIL_H_ */
