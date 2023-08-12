// Copyright © 2016 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <cstring>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

#include "python.h"

#ifdef BUSTER
#include <experimental/string_view>
using namespace std::experimental;
#endif

using namespace std;

vector<string> versions;

void init_python()
{
	DIR *dp;
	struct dirent *dirp;
	dp = opendir("/usr/bin");
	while ((dirp = readdir(dp)) != nullptr) {
		string entry { dirp->d_name };
		if (entry.rfind("python3.") == 0 && entry.find("-") == string::npos) {
			string pyc_ver = "3" + entry.substr(8);
			versions.emplace_back(pyc_ver);
		}
	}
	sort(versions.begin(), versions.end());
}

static bool ends_with(string_view str, string_view suffix)
{
    if (suffix.size() > str.size())
		return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

bool pyc_has_py(string pyc, bool debug)
{
	if (pyc.length() < 15)
		return false;

	// #366616: Don't report .pyc files if a .py file exist

	// also ignore __pycache__ dirs
	// if .py are found in the same directory
	if (ends_with(pyc, "/__pycache__")) {
		string dir;
		DIR *dp;
		struct dirent *dirp;
		dir = pyc.substr(0, pyc.length()-12);
		dp = opendir(dir.c_str());
		if(dp == nullptr) {
			cerr << "Failed to open directory " << dir << ": " << strerror(errno) << endl;
			return false;
		}
		while ((dirp = readdir(dp)) != nullptr) {
			string_view entry { dirp->d_name };
			if (entry.length() < 4)
				continue;
			//cerr << ' ' << entry << endl;
			if (ends_with(entry, ".py")) {
				if (debug) cerr << "match: " << dir << '/' << entry << endl;
				closedir(dp);
				return true;
			}
		}
		closedir(dp);
		return false;
	}

	if (!ends_with(pyc, ".pyc"))
		return false;

	struct stat buffer;

#ifdef BUSTER
	// python2 support
	string py;
	py = pyc.substr(0, pyc.length()-1);
	if (stat(py.c_str(), &buffer) == 0) {
		if (debug) cerr << "match: " << py << endl;
		return true;
	}
#endif

	/* TODO: consider old .pyc from old uinstalled Python3.x as leftover garbage
	$ py3versions -s
	python3.11
	tchet@brix ~/git/cruft-ng $ grep ^supported-versions /usr/share/python3/debian_defaults
	supported-versions = python3.11

	take 3.11 as a baseline but also support the next expected version 3.12

	$ ls /usr/bin/python3\.* | grep 3\.[0-9]*$
	/usr/bin/python3.11
	/usr/bin/python3.12
	*/

	/*
	/usr/share/pgcli/pgcli/packages/counter.py
	/usr/share/pgcli/pgcli/packages/__pycache__/counter.cpython-34.pyc
	*/
	size_t pos = pyc.find("/__pycache__/");
	if (pos == string::npos)
		return false;
	pyc.replace(pos, 13, "/");

	pos = pyc.find(".cpython-");
	if (pos == string::npos)
		return false;

	bool installed = false;
	for (unsigned int i=0;i<versions.size();i++) {
		if(pyc.find(versions[i], pos) != string::npos) {
			installed = true;
			break;
		}
	}
	if (!installed) return false;

	int ugly=0;
        if(pyc.find(".cpython-310") != string::npos) ugly=1;
        if(pyc.find(".cpython-311") != string::npos) ugly=1;
        if(pyc.find(".cpython-312") != string::npos) ugly=1;
        if(pyc.find(".cpython-313") != string::npos) ugly=1;
	pyc.replace(pos, 15+ugly, ".py");

	bool matched;
	matched = (stat(pyc.c_str(), &buffer) == 0);
	if (matched && debug) cerr << "match: " << pyc << endl;
	return matched;
}
