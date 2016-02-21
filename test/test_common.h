/*
 * test_common.h
 *
 *  Created on: Nov 25, 2015
 *      Author: netmind
 */

#ifndef TEST_TEST_COMMON_H_
#define TEST_TEST_COMMON_H_

#include "../cahttp/ext/nmdutil/etcutil.h"
#define FDCHK_S(A) int _fdchk_s_##A = cahttpu::get_num_fds()
#define FDCHK_E(A) int _fdchk_e_##A = cahttpu::get_num_fds(); assert(_fdchk_s_##A == _fdchk_e_##A)

//#define FDCHK_S(A) int FDCHK_S_##_A = get_num_fds();
//#define FDCHK_E(A) int FDCHK_E_##_A = get_num_fds(); assert(_sfd == _efd);
//int get_num_fds();



#endif /* TEST_TEST_COMMON_H_ */
