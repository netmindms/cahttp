/*
 * alog.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */

#if 0
#include "flog.h"
#include <vector>
#include <chrono>
#include <sys/time.h>
#include <pthread.h>
using namespace std;
namespace cahttp {
static __thread char _gTimeBuf[30];


_LogInst gLogInst(LOG_DEBUG);

const char* GetLogTimeNow() {
	struct timeval tv;
	gettimeofday(&tv, (struct timezone*)nullptr);
	struct tm ttm;
	localtime_r(&tv.tv_sec, &ttm);
	sprintf(_gTimeBuf, "%02d:%02d:%02d.%03d", ttm.tm_hour, ttm.tm_min, ttm.tm_sec, (int)(tv.tv_usec/1000));
	return _gTimeBuf;
}

//
//void print_log(const char *format, fmt::ArgList args) {
//  fmt::print(format, args);
//}
//FMT_VARIADIC(void, report_error, const char *)

void prtlog(const char *fmtstr, ...)
{
#if 1
#else
	struct timeval tm;
	va_list ap;

	gettimeofday(&tm, NULL);
	struct tm* ptr_time = localtime(&tm.tv_sec);

	char buf[TEMP_LOG_BUF_SIZE];
	va_start(ap, fmtstr);
	vsnprintf(buf, TEMP_LOG_BUF_SIZE-1, fmtstr, ap);
	va_end(ap);

	printf("%02d:%02d:%02d.%02d [%s]:%-5d %s\n", ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec, (int)(tm.tv_usec/10000), tagid, line, buf);
#endif
}

void CaHttpSetLogLevel(int l) {
	gLogInst.setLogLevel(l);
}
}
#endif
