#include <iostream>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "python.h"

using namespace std;

bool pyc_has_py(string pyc, bool debug)
{
	if (pyc.length() < 15)
		return false;

	// #366616: Don't report .pyc files if a .py file exist

	// also ignore __pycache__ dirs
	// if .py are found in the same directory
	if (pyc.substr(pyc.length()-12, 12) == "/__pycache__") {
                string dir;
		DIR *dp;
		struct dirent *dirp;
                dir = pyc.substr(0, pyc.length()-12);
		if((dp = opendir(dir.c_str())) == NULL) {
			return false;
		}
		while ((dirp = readdir(dp)) != NULL) {
			string entry = dirp->d_name;
			if (entry.length() < 4)
				continue;
                        //cerr << ' ' << entry << endl;
			if (entry.substr(entry.length()-3,3) == ".py") {
				if (debug) cerr << "match: " << dir << '/' << entry << endl;
				return true;
			}
		}
		return false;
	}

	/* scenario 1:
	/usr/share/python/debpython/debhelper.py
	/usr/share/python/debpython/debhelper.pyc
	*/
	if (pyc.substr(pyc.length()-4, 4) != ".pyc")
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
