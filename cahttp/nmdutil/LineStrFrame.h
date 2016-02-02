/*
 * LineStrFrame.h
 *
 *  Created on: Mar 28, 2015
 *      Author: netmind
 */

#ifndef UTIL_LINESTRFRAME_H_
#define UTIL_LINESTRFRAME_H_

#include <string>
#include <list>

using namespace std;

namespace nmdu {

class LineStrFrame
{
public:
	LineStrFrame();
	virtual ~LineStrFrame();
	int feedPacket(const char* buf, size_t len);
	string getLine();
	list<string> getAllLine();
	void close();

private:
	list<string> mLineList;
};

}

#endif /* UTIL_LINESTRFRAME_H_ */
