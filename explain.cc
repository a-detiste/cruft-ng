// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstring>
#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "explain.h"
#include "usr_merge.h"
#include "owner.h"

static void read_one_explain(const string& script, const string& package, vector<owner>& explain)
{
	int fd[2];
	if (pipe(fd) != 0) {
		perror("pipe");
		exit(1);
	}
	if(!fork()) // child
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execl(script.c_str(), script.c_str(), static_cast<char*>(nullptr));
		cerr << "Failed to execute command '" << script.c_str() << "': " << strerror(errno) << endl;
		exit(1);
	}

	close(fd[1]);
	FILE* fp;
	fp=fdopen(fd[0], "r");
	char buf[4096];
	string filter;
	string real_package = package;
	while (fgets(buf, sizeof(buf),fp))
	{
		filter=buf;
		filter=filter.substr(0,filter.size() - 1); // remove '/n'
		if (filter.front() == '/') {
			explain.emplace_back(real_package, usr_merge(filter));
		} else {
			real_package = filter;
		}
	}
	fclose(fp);
}

static void read_uppercase(vector<owner>& explain, const string& directory, bool debug)
{
	DIR *dp;
	struct dirent *dirp;

	if (debug) cerr << "EXECUTING UPPERCASE FILTERS IN " << directory  << endl;

	if((dp = opendir(directory.c_str())) == nullptr) {
		cerr << "Failed to open directory " << directory << ": " << strerror(errno) << endl;
		return;
	}
	while ((dirp = readdir(dp)) != nullptr) {
		string package = dirp->d_name;
		if (package==".") continue;
		if (package=="..") continue;
		if (!any_of(package.begin(), package.end(), [] (unsigned char c) { return islower(c); }))
			read_one_explain(directory + package, package, explain);
	}
	closedir(dp);
	if (debug) cerr << endl;
}

int read_explain(const string& dir, const vector<string>& packages, vector<owner>& explain)
{
	bool debug=getenv("DEBUG") != nullptr;

	read_uppercase(explain, "/usr/libexec/cruft/", debug);
	read_uppercase(explain, dir, debug);

	if (debug) cerr << "EXECUTING OTHER FILTERS" << endl;
	for (const auto& package: packages) {
		struct stat stat_buffer;
		string etc_filename = dir + package;
		string usr_filename = "/usr/libexec/cruft/" + package;
		if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
			read_one_explain(etc_filename, package, explain);
		else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
			read_one_explain(usr_filename, package, explain);
	}
	sort(explain.begin(), explain.end());
	explain.erase( unique( explain.begin(), explain.end() ), explain.end() );
	return 0;
}
