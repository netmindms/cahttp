/*
 * netutil.h
 *
 *  Created on: Jul 17, 2015
 *      Author: root
 */

#ifndef EXTERNAL_UTIL_NETUTIL_H_
#define EXTERNAL_UTIL_NETUTIL_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

namespace nmdu {

std::string Ip2Str(uint32_t ip);
const char* Ip2CStr(uint32_t ip);
uint32_t GetIpAddrByHostName(const std::string& hostname);
}

#endif /* EXTERNAL_UTIL_NETUTIL_H_ */
