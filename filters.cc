#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#include "filters.h"
#include "usr_merge.h"

bool owner_sorter(owner const& l, owner const& r)
{
    return l.glob < r.glob;
}

void read_one_filter(const string& glob_filename, const string& package, vector<owner>& globs)
{
	bool debug=getenv("DEBUG_RULES") != NULL;

	if (debug) cerr << "READING " << glob_filename << endl;
	ifstream glob_file(glob_filename.c_str());
	while (glob_file.good())
	{
		string glob_line;
		getline(glob_file,glob_line);
		if (glob_file.eof()) break;
		if (glob_line.substr(0,1) == "/") {
			owner glob(package, usr_merge(glob_line));
			globs.push_back(glob);
			if (debug) cerr << package << " " << glob_line << endl;
		}
	}
	glob_file.close();
}

int read_filters(/* const */ vector<string>& packages, vector<owner>& globs)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "READING UPERCASE GLOBS IN /etc/cruft/filters/" << endl;
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/etc/cruft/filters/")) == NULL) {
	      cerr << "Error(" << errno << ") opening /etc/cruft/filters/" << endl;
	      exit(1);
	}
	while ((dirp = readdir(dp)) != NULL) {
		string package=string(dirp->d_name);
		if (package == "." or package == "..") continue;
		string uppercase=package;
		transform(uppercase.begin(), uppercase.end(), uppercase.begin(), ::toupper);
		if (package==uppercase)
			read_one_filter("/etc/cruft/filters/" + package, package, globs);
	}
	closedir(dp);
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;

	if (debug) cerr << "READING OTHER GLOBS " << endl;
	vector<string>::iterator it;

	for (it=packages.begin(); it!=packages.end(); it++) {
		string package=*it;
		struct stat stat_buffer;
		string etc_filename = "/etc/cruft/filters/" + package;
		string usr_filename = "/usr/lib/cruft/filters-unex/" + package; // should be empty
		if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(etc_filename, package, globs);
		else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(usr_filename, package, globs);
	}
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;

	if (debug) cerr << "READING MAIN RULE ARCHIVE " << endl;
	ifstream glob_file("/usr/share/cruft/ruleset");
	string etc_filename;
	struct stat stat_buffer;
	bool keep = false;
	string package;
	while (glob_file.good())
	{
		string glob_line;
		getline(glob_file,glob_line);
		if (glob_file.eof()) break;
		if (glob_line.substr(0,1) == "/") {
			if (keep) {
				owner glob(package, usr_merge(glob_line));
				globs.push_back(glob);
			}
		} else {
			// new package entry
			package = string(glob_line);
			etc_filename = "/etc/cruft/filters/" + package;
			keep = bool(find(packages.begin(), packages.end(), package) != packages.end()) & bool(!stat(etc_filename.c_str(), &stat_buffer)==0);
			//cerr << package << " " << keep << endl;
                }
	}
	glob_file.close();

	sort(globs.begin(), globs.end(), &owner_sorter);
	globs.erase( unique( globs.begin(), globs.end() ), globs.end() );
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;
	return 0;
}
