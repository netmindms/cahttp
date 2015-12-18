/*
 * CmdOpt.cpp
 *
 *  Created on: Apr 3, 2015
 *      Author: netmind
 */

#include "CmdOpt.h"


static string __NULLSTR;

CmdOpt::CmdOpt()
{
	// TODO Auto-generated constructor stub

}

CmdOpt::~CmdOpt()
{
	// TODO Auto-generated destructor stub
}

void CmdOpt::parse(int argc, char* argv[])
{
	opterr = 0;
	for (;;)
	{
		int c = getopt(argc, argv, mOptStr.data());
		if (c > 0)
		{
			if (c != '?')
			{
				string s;
				s.push_back(c);
				auto &arg = mOptMap[s];
				if (optarg)
				{
					arg.first = 1;
					arg.second.push_back(optarg);
				}
				else
				{
					arg.first = 0;
				}
			}
		}
		else
		{
			break;
		}
	}

	for (auto i = optind; i < argc; i++)
		mArgList.push_back(argv[i]);

}

const string& CmdOpt::getOptionVal(char ks)
{
	try
	{
		auto &arg = mOptMap.at(string(1, ks));
		if (arg.first)
		{
			return arg.second[0];
		}
		else
		{
			return __NULLSTR;
		}
	} catch (out_of_range &err)
	{
		return __NULLSTR;
	}

}

void CmdOpt::addOption(const string &os)
{
	mOptStr += os;
}

void CmdOpt::addOptionVal(const string &os)
{
	for (size_t i = 0; i < os.size(); i++)
	{
		mOptStr += string(1, os[i]) + ":";
	}
}

bool CmdOpt::checkOption(char ks)
{
	auto itr = mOptMap.find(string(1, ks));
	if (itr != mOptMap.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

const string& CmdOpt::getArg(int i)
{
	if(mArgList.size()>0)
		return mArgList[i];
	else
		return __NULLSTR;
}

const vector<string>& CmdOpt::getArgAll()
{
	return mArgList;
}

vector<string> CmdOpt::fetchArgAll()
{
	return move(mArgList);
}

