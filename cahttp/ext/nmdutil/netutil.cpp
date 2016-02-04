/*
 * netutil.cpp
 *
 *  Created on: Jul 17, 2015
 *      Author: root
 */
#include <string>
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include "netutil.h"

using namespace std;

namespace cahttpu {

std::string Ip2Str(uint32_t ip) {
	return std::string(Ip2CStr(ip));
}

const char* Ip2CStr(uint32_t ip) {
	struct in_addr in;
	in.s_addr = ip;
	return inet_ntoa(in);
}

uint32_t GetIpAddrByHostName(const string& hostname) {
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ((he = gethostbyname(hostname.data())) == NULL) {
		return 0;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for (i = 0; addr_list[i] != NULL; i++) {
		return (*addr_list[i]).s_addr;
	}

	return 0;
}

}
