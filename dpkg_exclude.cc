// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include <fstream>
#include <dirent.h>

#include "dpkg_exclude.h"

static int read_one_cfg(const string& filename, vector <string>& globs)
{
	ifstream glob_file(filename);
	for (string glob_line; getline(glob_file,glob_line);)
	{
		if (glob_line.find("path-exclude") != 0) continue;
		glob_line = glob_line.substr(glob_line.find('/'));
                // translate regular globs into cruft ones
		if (glob_line.back() == '*')
			glob_line += '*';
		globs.emplace_back(glob_line);
	}
	return 0;
}

int read_dpkg_excludes(vector<string>& excludes)
{
	read_one_cfg("/etc/dpkg/dpkg.cfg", excludes);

	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/etc/dpkg/dpkg.cfg.d/")) == nullptr) {
	      cerr << "Error(" << errno << ") opening /etc/dpkg/dpkg.cfg.d/" << endl;
	      return 1;
	}
	while ((dirp = readdir(dp)) != nullptr) {
		string cfg = dirp->d_name;
		if (cfg == "." or cfg == "..") continue;
		read_one_cfg("/etc/dpkg/dpkg.cfg.d/" + cfg, excludes);
	}
	closedir(dp);
	return 0;
}
