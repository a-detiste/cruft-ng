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
		if (glob_line.front() == '/') {
			if (debug) cerr << package << " " << glob_line << endl;
			globs.emplace_back(package, usr_merge(glob_line));
		}
	}
}

int read_filters(const vector<string>& packages, vector<owner>& globs)
{
	bool debug=getenv("DEBUG") != nullptr;

	if (debug) cerr << "READING UPERCASE GLOBS IN /etc/cruft/filters/" << endl;
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/etc/cruft/filters/")) == nullptr) {
	      cerr << "Error(" << errno << ") opening /etc/cruft/filters/" << endl;
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
		string etc_filename = "/etc/cruft/filters/" + package;
		string usr_filename = "/usr/lib/cruft/filters-unex/" + package; // should be empty
		if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(etc_filename, package, globs, debug);
		else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(usr_filename, package, globs, debug);
	}
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;

	if (debug) cerr << "READING MAIN RULE ARCHIVE " << endl;
	ifstream glob_file("/usr/share/cruft/ruleset");
	bool keep = false;
	string package;
	for (string glob_line; getline(glob_file, glob_line);)
	{
		if (glob_line.empty())
			continue;
		if (glob_line.front() == '/') {
			if (keep) {
				owner glob(package, usr_merge(glob_line));
				globs.push_back(glob);
			}
		} else {
			// new package entry
			package = glob_line;
			string etc_filename = "/etc/cruft/filters/" + package;
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
