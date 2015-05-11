#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>

#include "filters.h"

int read_filters(/* const */ vector<string>& packages, vector<string>& globs)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "READING GLOBS IN /usr/lib/cruft/filters-unex/" << endl;
	vector<string>::iterator it=packages.begin();

	string retain;
	for (;it !=packages.end();it++) {
		string package=*it;
		size_t arch=package.find(":");
		if (arch != string::npos ) package=package.substr(0,arch);
		if (package==retain) continue;
		retain=package;

		struct stat stat_buffer;
		string glob_filename;
		if ( stat(("/etc/cruft/filters/" + package).c_str(), &stat_buffer)==0 )
			glob_filename="/etc/cruft/filters/" + package;
		else if ( stat(("/usr/lib/cruft/filters-unex/" + package).c_str(), &stat_buffer)==0 )
			glob_filename ="/usr/lib/cruft/filters-unex/" + package;
		else continue;

		ifstream glob_file(glob_filename.c_str());
		while (glob_file.good())
		{
			string glob_line;
			getline(glob_file,glob_line);
			if (glob_file.eof()) break;
			if (glob_line.substr(0,1) == "/") globs.push_back(glob_line);
		}
		glob_file.close();
	}
	sort(globs.begin(), globs.end());
	if (debug) cerr << globs.size() << " globs in database" << endl << endl;
	// !!! TODO: remove duplicates
	return 0;
}
