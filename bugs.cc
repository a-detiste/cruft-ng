// Copyright © 2022 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

#include "owner.h"
#include "bugs.h"

using namespace std;


bug::bug( std::string package_, std::string bugno_ )
{
        package = std::move(package_);
        bugno = std::move(bugno_);
}

bool operator<(bug const& l, bug const &r)
{
    return l.bugno < r.bugno;
}


static void error(const string& bugs_path, const string& bugs_line)
{
	cerr << bugs_path << " is corrupted here: " << bugs_line << '\n';
}

void read_bugs(map<string, bug>& bugs, const string& bugs_path)
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
		bug entry(package, bug_nr);
		bugs.emplace(std::move(path), std::move(entry));
	}
}

#ifdef UNIT_TEST
//clang++ -DUNIT_TEST bugs.cc owner.cc -o test_bugs && ./test_bugs
int main()
{
	map<string, bug> bugs;
	read_bugs(bugs, "bugs");
	for(const auto& bug: bugs)
	{
	    std::cout << bug.first << ' ' << bug.second.package << ' ' << bug.second.bugno << '\n';
	}
}
#endif
