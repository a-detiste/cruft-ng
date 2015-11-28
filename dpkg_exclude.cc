#include <iostream>
#include <fstream>
#include <dirent.h>
#include <errno.h>

#include "dpkg_exclude.h"

int read_one_cfg(const string& filename, vector <string>& globs)
{
	ifstream glob_file(filename.c_str());
	while (glob_file.good())
	{
		string glob_line;
		getline(glob_file,glob_line);
		if (glob_file.eof()) break;
		if (glob_line.find("path-exclude") != 0) continue;
		glob_line = glob_line.substr(glob_line.find('/'));
                // translate regular globs into cruft ones
		if (glob_line.substr(glob_line.length()-1,1) == "*")
			glob_line += '*';
		globs.push_back(glob_line);
	}
	return 0;
}

int read_dpkg_excludes(vector<string>& globs)
{
	read_one_cfg("/etc/dpkg/dpkg.cfg", globs);

	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/etc/dpkg/dpkg.cfg.d/")) == NULL) {
	      cerr << "Error(" << errno << ") opening /etc/dpkg/dpkg.cfg.d/" << endl;
	      return 1;
	}
	while ((dirp = readdir(dp)) != NULL) {
		string cfg = string(dirp->d_name);
		if (cfg == "." or cfg == "..") continue;
		read_one_cfg("/etc/dpkg/dpkg.cfg.d/" + cfg, globs);
	}
	return 0;
}
