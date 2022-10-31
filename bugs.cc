#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

#include "owner.h"
#include "bugs.h"

using namespace std;


void error(const string& bugs_path, const string& bugs_line)
{
	cerr << bugs_path << " is corrupted here: " << bugs_line << endl;
}

static void read_bugs(map<string, owner>& bugs, const string& bugs_path)
{
	ifstream bugs_file(bugs_path);
	if (!bugs_file.is_open())
        	bugs_file.open("/usr/share/cruft/bugs");

	for (string bugs_line; getline(bugs_file, bugs_line);)
	{
		if (bugs_line.empty()) continue;
		stringstream ss (bugs_line);
		string path, bug_nr, package;
		if(!getline (ss, path, ' ')) return error(bugs_path, bugs_line);
		if(!getline (ss, bug_nr, ' ')) return error(bugs_path, bugs_line);
		if(!getline (ss, package, ' ')) return error(bugs_path, bugs_line);
		owner bug(package, package, bug_nr);
		bugs.insert(pair<string, owner>(path, bug));
	}
}

#ifdef UNIT_TEST
//clang++ -DUNIT_TEST bugs.cc owner.o -o test_bugs && ./test_bugs
int main()
{
	map<string, owner> bugs;
        read_bugs(bugs, "bugs");
	for(auto it = bugs.cbegin(); it != bugs.cend(); ++it)
	{
	    std::cout << it->first << ' ' << it->second.package << ' ' << it->second.bug << endl;
	}
}
#endif
