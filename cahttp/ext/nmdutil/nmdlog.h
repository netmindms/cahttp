/*
 * nmdlog.h
 *
 *  Created on: Apr 13, 2015
 *      Author: netmind
 */

#ifndef NMDLOG_H_
#define NMDLOG_H_


/* add follwing in compile option
 *  NMDU_FILE_NAME="\"$(<F)\""
 *
 */


/* Insert below lines in your app log file
 * In here, gYourLogInst is an instance pointer of cahttpu::LogInst

**** Sample Application log header(your_log.h) *****
#ifndef LOCAL_LOG_INST
#define LOCAL_LOG_INST your_log_instance_name // LogInst instance pointer
//#define LOCAL_LOG_INST DEFAULT_LOG_INST
#endif
#include <nmdutil/nmdlog.h>
EXTERN_LOG_INSTANCE(your_log_instance_name); //==> extern cahttpu::LogInst* your_log_instance_name;


**** Sample Application log source(your_log.cpp) *****
#include "your_log.h"
MAKE_LOG_INSTANCE(your_log_instance_name);

*/

#include <memory>
#include <string>
#include <mutex>
#include <unistd.h>
#include "nmdu_format.h"
namespace cahttpu {

#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_WARN 2
#define LOG_NOTICE 3
#define LOG_INFO 4
#define LOG_DEBUG 5
#define LOG_VERBOSE 6


#ifndef NMDU_FILE_NAME
#define NMDU_FILE_NAME "Line"
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

#define DEFAULT_LOG_INST (cahttpu::_gDefLogInst)

class LogInst {
public:
	LogInst();
	virtual ~LogInst();
	void lock();
	void unlock();
	void level(int level) {
		mLevel = level;
	}

	int level() {
		return mLevel;
	}
	void levelFile(int level) {
		mFileLevel = level;
	}
	int levelFile() {
		return mFileLevel;
	}
	int setLogFile(const char* path, size_t max=10*1024*1024);
	int getFileFd() {
		return mFd;
	}
	inline void writeFile(const char* ptr, size_t len) {
		lock();
		write(mFd, ptr, len);
		checkSize();
		unlock();
	};
	inline void writeLog(FILE* scr_st, const char* ptr, size_t len) {
		lock();
		std::fwrite(ptr, 1, len, scr_st);
		unlock();
	};
private:
	std::mutex mMutex;
	int mLevel;
	int mFileLevel;
	int mFd;
	std::string mFilePath;
	std::string mBackupFolder;
	ssize_t mMaxFileSize;

	void checkSize();
	int backupFile();
};



extern LogInst* _gDefLogInst;
std::string GetLogTimeNow();
LogInst* getDefLogInst();

//void NmduMemPrintf(cahttpu::fmt::MemoryWriter& w, cahttpu::fmt::StringRef sref, cahttpu::fmt::ArgList args);
inline void NmduMemPrintf(cahttpu::fmt::MemoryWriter& w, cahttpu::fmt::StringRef sref, cahttpu::fmt::ArgList args) {
	fmt::printf(w, sref, args);
}
FMT_VARIADIC(void, NmduMemPrintf, cahttpu::fmt::MemoryWriter&, cahttpu::fmt::StringRef)

#define MAKE_LOG_INSTANCE(NAME) \
cahttpu::LogInst *NAME=nullptr; \
struct INST_TYPE_##NAME { \
	INST_TYPE_##NAME() { \
		NAME = new cahttpu::LogInst; \
	};\
	virtual ~INST_TYPE_##NAME() { \
		if(NAME) delete (NAME); \
	};\
}; static INST_TYPE_##NAME _MOD_INIT_##NAME;

#define EXTERN_LOG_INSTANCE(LOG) extern cahttpu::LogInst* LOG;

#ifndef LOCAL_LOG_INST
#define LOCAL_LOG_INST DEFAULT_LOG_INST
#endif

#define C_LOG_ERROR "E"
#define C_LOG_WARN "W"
#define C_LOG_NOTICE "N"
#define C_LOG_INFO "I"
#define C_LOG_DEBUG "D"
#define C_LOG_VERBOSE "V"

#ifdef LOCAL_LOG_INST
#define NMDU_SET_LOG_LEVEL(L) LOCAL_LOG_INST->level(L)
#define NMDU_SET_LOG_LEVEL_FILE(L) LOCAL_LOG_INST->levelFile(L)
#define NMDU_SET_LOG_FILE(F) LOCAL_LOG_INST->setLogFile(F)
#if 1
#define NMDULOGPUT(LL, OUT, FMTSTR, ...) do {\
	if(LOG_LEVEL>=LL) {\
		cahttpu::fmt::MemoryWriter w;\
		if(LOCAL_LOG_INST->level()>=LL) {\
			cahttpu::NmduMemPrintf(w, "%s [" C_ ##LL ":%s:%d] " FMTSTR "\n", \
					cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);\
			/*std::fwrite(w.data(), 1, w.size(), OUT);\*/ \
			LOCAL_LOG_INST->writeLog(OUT, w.data(), w.size()); \
		}\
		if(LOCAL_LOG_INST->levelFile()>=LL) {\
			LOCAL_LOG_INST->writeFile(w.data(), w.size());\
		}\
	}\
	}while(0)
#endif
#define ale(FMTSTR, ...) NMDULOGPUT(LOG_ERROR, stderr, FMTSTR, ## __VA_ARGS__)
#define alw(FMTSTR, ...) NMDULOGPUT(LOG_WARN, stderr, FMTSTR, ## __VA_ARGS__)
#define aln(FMTSTR, ...) NMDULOGPUT(LOG_NOTICE, stdout, FMTSTR, ## __VA_ARGS__)
#define ali(FMTSTR, ...) NMDULOGPUT(LOG_INFO, stdout, FMTSTR, ## __VA_ARGS__)
#define ald(FMTSTR, ...) NMDULOGPUT(LOG_DEBUG, stdout, FMTSTR, ## __VA_ARGS__)
#define alv(FMTSTR, ...) NMDULOGPUT(LOG_VERBOSE, stdout, FMTSTR, ## __VA_ARGS__)
#else
#define ale(FMTSTR, ...) { if(LOG_LEVEL>=LOG_ERROR) { if(LOCAL_LOG_INST->level()>=LOG_ERROR) { LOCAL_LOG_INST->lock();cahttpu::fmt::fprintf(stderr, "%s [E:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->unlock(); }; if(LOCAL_LOG_INST->levelFile()>=LOG_ERROR) {LOCAL_LOG_INST->lock(); std::string s = cahttpu::fmt::sprintf("%s [E:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->writeFile(s.data(), s.size());LOCAL_LOG_INST->unlock();} } }
#define alw(FMTSTR, ...) { if(LOG_LEVEL>=LOG_WARN) { if(LOCAL_LOG_INST->level()>=LOG_WARN) { LOCAL_LOG_INST->lock();cahttpu::fmt::fprintf(stderr, "%s [W:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->unlock(); }; if(LOCAL_LOG_INST->levelFile()>=LOG_WARN) {LOCAL_LOG_INST->lock(); std::string s = cahttpu::fmt::sprintf("%s [W:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->writeFile(s.data(), s.size());LOCAL_LOG_INST->unlock();} } }
#define aln(FMTSTR, ...) { if(LOG_LEVEL>=LOG_NOTICE) { if(LOCAL_LOG_INST->level()>=LOG_NOTICE) { LOCAL_LOG_INST->lock();cahttpu::fmt::printf("%s [N:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->unlock(); }; if(LOCAL_LOG_INST->levelFile()>=LOG_NOTICE) {LOCAL_LOG_INST->lock(); std::string s = cahttpu::fmt::sprintf("%s [N:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->writeFile(s.data(), s.size());LOCAL_LOG_INST->unlock();} } }
#define ali(FMTSTR, ...) { if(LOG_LEVEL>=LOG_INFO) { if(LOCAL_LOG_INST->level()>=LOG_INFO) { LOCAL_LOG_INST->lock();cahttpu::fmt::printf("%s [I:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->unlock(); }; if(LOCAL_LOG_INST->levelFile()>=LOG_INFO) {LOCAL_LOG_INST->lock(); std::string s = cahttpu::fmt::sprintf("%s [I:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->writeFile(s.data(), s.size());LOCAL_LOG_INST->unlock();} } }
#define ald(FMTSTR, ...) { if(LOG_LEVEL>=LOG_DEBUG) { if(LOCAL_LOG_INST->level()>=LOG_DEBUG) { LOCAL_LOG_INST->lock();cahttpu::fmt::printf("%s [D:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->unlock(); }; if(LOCAL_LOG_INST->levelFile()>=LOG_DEBUG) {LOCAL_LOG_INST->lock(); std::string s = cahttpu::fmt::sprintf("%s [D:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->writeFile(s.data(), s.size());LOCAL_LOG_INST->unlock();} } }
#define alv(FMTSTR, ...) { if(LOG_LEVEL>=LOG_VERBOSE) { if(LOCAL_LOG_INST->level()>=LOG_VERBOSE) { LOCAL_LOG_INST->lock();cahttpu::fmt::printf("%s [V:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->unlock(); }; if(LOCAL_LOG_INST->levelFile()>=LOG_VERBOSE) {LOCAL_LOG_INST->lock(); std::string s = cahttpu::fmt::sprintf("%s [V:%s:%d] " FMTSTR "\n", cahttpu::GetLogTimeNow(), NMDU_FILE_NAME, __LINE__, ## __VA_ARGS__);LOCAL_LOG_INST->writeFile(s.data(), s.size());LOCAL_LOG_INST->unlock();} } }
#endif
}

#endif /* NMDLOG_H_ */
