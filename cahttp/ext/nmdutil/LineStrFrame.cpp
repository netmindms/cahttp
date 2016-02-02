/*
 * LineStrFrame.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: netmind
 */

#include <iostream>
#include <string.h>
#include "LineStrFrame.h"


#include "strutil.h"

namespace nmdu {

LineStrFrame::LineStrFrame()
{
	// TODO Auto-generated constructor stub

}

LineStrFrame::~LineStrFrame()
{
	// TODO Auto-generated destructor stub
}

int LineStrFrame::feedPacket(const char* buf, size_t len)
{
	size_t remain = len;
	const char* ptr = buf;
	for(;remain>0;)
	{
		char* endp = (char*)memchr(ptr, '\n', remain);
		if(endp != nullptr) {
			auto rcnt = endp-ptr;
			string ts(ptr, rcnt);
			cout << "parse lien: " << ts.data() << endl;
			Ltrim(Rtrim(ts));
			if(ts.empty()==false)
				mLineList.push_back(move(ts));
			rcnt++;
			ptr += rcnt;
			remain -= rcnt;
		} else {
			string ts(ptr, remain);
			Ltrim(Rtrim(ts));
			if(ts.empty() == false)
				mLineList.push_back(move(ts));
			remain = 0;
		}
	}
	return mLineList.size();
}

string LineStrFrame::getLine()
{
	if(mLineList.empty() == false) {
		auto s = move(mLineList.front());
		mLineList.pop_front();
		return move(s);
	} else {
		return "";
	}
}

list<string> LineStrFrame::getAllLine()
{
	return move(mLineList);
}

void LineStrFrame::close()
{
	mLineList.clear();
}

}

#if 0
#include <gtest/gtest.h>
TEST(linestr, frame)
{
	string ts ="1111\r\n2222\n\r\nads\n232\n";
	list<string> orglines = {"1111", "2222", "ads", "232"};
	LineStrFrame frame;
	auto ret = frame.feedPacket(ts.data(), ts.size());
	ASSERT_EQ(ret, 4);
	auto lines = frame.getAllLine();
	ASSERT_EQ(orglines, lines);

	int i=-1;
	for(auto &l: lines) {
		i++;
		cout << "[" << i << "] " << l << endl;
	}
}
#endif
