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

	/* scenario 1:
	/usr/share/python/debpython/debhelper.py
	/usr/share/python/debpython/debhelper.pyc
	*/
	if (!ends_with(pyc, ".pyc"))
		return false;

	struct stat buffer;
        string py;
	py = pyc.substr(0, pyc.length()-1);
	if (stat(py.c_str(), &buffer) == 0) {
		if (debug) cerr << "match: " << py << endl;
		return true;
	}

	/* scenario 2:
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
	int ugly=0;
        if(pyc.find(".cpython-310") != string::npos) ugly=1;
        if(pyc.find(".cpython-311") != string::npos) ugly=1;
	pyc.replace(pos, 15+ugly, ".py");

        bool matched;
        matched = (stat(pyc.c_str(), &buffer) == 0);
        if (matched && debug) cerr << "match: " << pyc << endl;
        return matched;
}
