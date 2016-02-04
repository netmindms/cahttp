/*
 * CmdOpt.h
 *
 *  Created on: Apr 3, 2015
 *      Author: netmind
 */

#ifndef UTIL_CMDOPT_H_
#define UTIL_CMDOPT_H_
#include <stdexcept>
#include <unistd.h>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

namespace cahttpu {

typedef pair<int, vector<string> > opt_arg;

class CmdOpt
{
public:
	CmdOpt();
	virtual ~CmdOpt();
	void addOption(const string& c);
	void addOptionVal(const string& c);
	void parse(int argc, char* argv[]);

	const string& getOptionVal(char ks);
	bool checkOption(char ks);
	const string& getArg(int i=0);
	const vector<string>& getArgAll();
	vector<string> fetchArgAll();

private:
	string mOptStr;
	unordered_map<string, opt_arg> mOptMap;
	vector<string> mArgList;
};

}
#endif /* UTIL_CMDOPT_H_ */
