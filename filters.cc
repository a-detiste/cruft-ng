// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#include "filters.h"
#include "usr_merge.h"

static void read_one_filter(const string& glob_filename, const string& package, vector<owner>& globs, bool debug)
{
	if (debug) cerr << "READING " << glob_filename << endl;
	ifstream glob_file(glob_filename);
	for (string glob_line; getline(glob_file, glob_line);)
	{
		if (glob_line.empty()) continue;
		if (glob_line.front() == '#') continue;
		if (glob_line.front() == '/') {
			if (debug) cerr << package << " " << glob_line << endl;
			globs.emplace_back(package, usr_merge(glob_line));
		}
	}
}

int read_filters(const string& dir, const string& ruleset_file, const vector<string>& packages, vector<owner>& globs)
{
	bool debug=getenv("DEBUG") != nullptr;

	if (debug) cerr << "READING UPPERCASE GLOBS IN " << dir << endl;
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(dir.c_str())) == nullptr) {
	      cerr << "Failed to open filters directory " << dir << ": " << strerror(errno) << endl;
	      exit(1);
	}
	while ((dirp = readdir(dp)) != nullptr) {
		string package = dirp->d_name;
		if (package == "." or package == "..") continue;

		if (!any_of(package.begin(), package.end(), [](unsigned char c){ return islower(c); }))
			read_one_filter(dir + package, package, globs, debug);
	}
	closedir(dp);
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;

	if (debug) cerr << "READING OTHER GLOBS " << endl;

	for (const auto& package : packages) {
		struct stat stat_buffer;
		string etc_filename = dir + package;
		string usr_filename = "/usr/lib/cruft/filters-unex/" + package;
		string usr_filename_new = "/usr/share/cruft/rules/" + package;
		if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(etc_filename, package, globs, debug);
		else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(usr_filename, package, globs, debug);
		else if ( stat(usr_filename_new.c_str(), &stat_buffer)==0 )
			read_one_filter(usr_filename_new, package, globs, debug);
	}
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;

	if (debug) cerr << "READING MAIN RULE ARCHIVE " << endl;
	ifstream glob_file(ruleset_file);
	bool keep = false;
	string package;
	for (string glob_line; getline(glob_file, glob_line);)
	{
		if (glob_line.empty())
			continue;
		if (glob_line.front() == '#')
			continue;
		if (glob_line.front() == '/') {
			if (keep) {
				globs.emplace_back(package, usr_merge(glob_line));
			}
		} else {
			// new package entry
			package = glob_line;
			string etc_filename = dir + package;
			struct stat stat_buffer;
			keep = find(packages.begin(), packages.end(), package) != packages.end() && stat(etc_filename.c_str(), &stat_buffer)!=0;
			//cerr << package << " " << keep << endl;
		}
	}
	glob_file.close();

	sort(globs.begin(), globs.end());
	globs.erase( unique( globs.begin(), globs.end() ), globs.end() );
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;
	return 0;
}
