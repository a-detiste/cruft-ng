#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#include "filters.h"
#include "usr_merge.h"

void read_one_filter(const string& glob_filename, vector<string>& globs)
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
			globs.push_back(usr_merge(glob_line));
			if (debug) cerr << glob_line << endl;
		}
	}
	glob_file.close();
}

int read_filters(/* const */ vector<string>& packages, vector<string>& globs)
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
			read_one_filter("/etc/cruft/filters/" + package, globs);
	}
	closedir(dp);
	if (debug) cerr << endl;


	if (debug) cerr << "READING OTHER GLOBS " << endl;
	vector<string>::iterator it=packages.begin();

	string retain;
	for (;it !=packages.end();it++) {
		string package=*it;
		size_t arch=package.find(":");
		if (arch != string::npos ) package=package.substr(0,arch);
		if (package==retain) continue;
		retain=package;

		struct stat stat_buffer;
		string etc_filename = "/etc/cruft/filters/" + package;
		string usr_filename = "/usr/lib/cruft/filters-unex/" + package; // should be empty
		if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(etc_filename, globs);
		else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
			read_one_filter(usr_filename, globs);
	}

	if (debug) cerr << "READING MAIN RULE ARCHIVE " << endl;
	ifstream glob_file("/usr/share/cruft/ruleset");
	string etc_filename;
	while (glob_file.good())
	{
		string glob_line;
		getline(glob_file,glob_line);
		if (glob_file.eof()) break;
		if (glob_line.substr(0,1) == "/") {
			struct stat stat_buffer;
			if ( !stat(etc_filename.c_str(), &stat_buffer)==0 ) {
				globs.push_back(usr_merge(glob_line));
			}
		} else {
			// new package entry
			etc_filename = "/etc/cruft/filters/" + glob_line;
                }
	}
	glob_file.close();

	sort(globs.begin(), globs.end());
	globs.erase( unique( globs.begin(), globs.end() ), globs.end() );
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;
	return 0;
}
