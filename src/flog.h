/*
 * alog.h
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */

#ifndef SRC_ALOG_H_
#define SRC_ALOG_H_

#include <memory>
#include "format.h"
using namespace std;
namespace cahttp {
//#include <syslog.h>
#define LOG_ERROR 0
#define LOG_WARN 1
#define LOG_NOTICE 2
#define LOG_INFO 3
#define LOG_DEBUG 4

/* add follwing in compile option
 *  MYPREP_BASE_FILE_NAME="\"$(<F)\""
 *
 */

class _LogInst {
public:
	_LogInst(int l) {
		level = l;
		pthread_mutex_init(&mMutex, nullptr);
	}
	~_LogInst() {
		pthread_mutex_destroy(&mMutex);
	}
	void lock() {
		pthread_mutex_lock(&mMutex);
	}
	void unlock() {
		pthread_mutex_unlock(&mMutex);
	}
	void setLogLevel(int l) {
		level = l;
	}

	int level;
private:
	pthread_mutex_t mMutex;
};

extern _LogInst gLogInst;
const char* GetLogTimeNow();


#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

#define ale(FMTSTR, ...) if(gLogInst.level>=LOG_ERROR) {gLogInst.lock();fmt::fprintf(stderr, "%s [%s:%d] " FMTSTR "\n", GetLogTimeNow(), MYPREP_BASE_FILE_NAME, __LINE__, ## __VA_ARGS__);gLogInst.unlock();};
#define alw(FMTSTR, ...) if(gLogInst.level>=LOG_WARN) {gLogInst.lock();fmt::fprintf(stderr, "%s [%s:%d] " FMTSTR "\n", GetLogTimeNow(), MYPREP_BASE_FILE_NAME, __LINE__, ## __VA_ARGS__);gLogInst.unlock();};
#define aln(FMTSTR, ...) if(LOG_LEVEL>=LOG_NOTICE) if(gLogInst.level>=LOG_NOTICE) {gLogInst.lock();fmt::printf("%s [%s:%d] " FMTSTR "\n", GetLogTimeNow(), MYPREP_BASE_FILE_NAME, __LINE__, ## __VA_ARGS__);gLogInst.unlock();}
#define ali(FMTSTR, ...) if(LOG_LEVEL>=LOG_INFO) if(gLogInst.level>=LOG_INFO) {gLogInst.lock();fmt::printf("%s [%s:%d] " FMTSTR "\n", GetLogTimeNow(), MYPREP_BASE_FILE_NAME, __LINE__, ## __VA_ARGS__);gLogInst.unlock();}
#define ald(FMTSTR, ...) if(LOG_LEVEL>=LOG_DEBUG) if(gLogInst.level>=LOG_DEBUG) {gLogInst.lock();fmt::printf("%s [%s:%d] " FMTSTR "\n", GetLogTimeNow(), MYPREP_BASE_FILE_NAME, __LINE__, ## __VA_ARGS__);gLogInst.unlock();}

void CaHttpSetLogLevel(int l);
}
#endif /* SRC_ALOG_H_ */
