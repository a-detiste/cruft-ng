#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "explain.h"
#include "usr_merge.h"
#include "owner.h"

void read_one_explain(const string& script, const string& package, vector<owner>& explain)
{
	int fd[2];
	if (pipe(fd) != 0) {
		perror("pipe");
		exit(1);
	};
	if(!fork()) // child
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execl(script.c_str(), script.c_str(), (char*) NULL);
		exit(1);
	}

	close(fd[1]);
	FILE* fp;
	fp=fdopen(fd[0], "r");
	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	string filter;
	while (fgets(buf, sizeof(buf),fp))
	{
		filter=buf;
		filter=filter.substr(0,filter.size() - 1); // remove '/n'
		//if (debug) cerr << "# " << filter << endl;
		owner seen(package, usr_merge(filter));
		explain.push_back(seen);
	}
	fclose(fp);
}

void read_uppercase(vector<owner>& explain, string directory)
{
	DIR *dp;
	struct dirent *dirp;

	bool debug=getenv("DEBUG") != NULL;
	if (debug) cerr << "EXECUTING UPPERCASE FILTERS IN " << directory  << endl;

	if((dp = opendir(directory.c_str())) == NULL) {
		cerr << "Error(" << errno << ") " << directory << endl;
	}
	while ((dirp = readdir(dp)) != NULL) {
		string package=string(dirp->d_name);
		if (package==".") continue;
		if (package=="..") continue;
		string uppercase=package;
		transform(uppercase.begin(), uppercase.end(), uppercase.begin(), ::toupper);
		if (package==uppercase)
			read_one_explain(directory + package, package, explain);
	}
	closedir(dp);
	if (debug) cerr << endl;
}

int read_explain(vector<string>& packages, vector<owner>& explain)
{
	bool debug=getenv("DEBUG") != NULL;

	read_uppercase(explain, "/usr/libexec/cruft/");
	read_uppercase(explain, "/etc/cruft/explain/");

	if (debug) cerr << "EXECUTING OTHER FILTERS" << endl;
	vector<string>::iterator package;
	for (package=packages.begin(); package !=packages.end(); package++) {
                struct stat stat_buffer;
                string etc_filename = "/etc/cruft/explain/" + *package;
                string usr_filename = "/usr/libexec/cruft/" + *package;
                if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
                        read_one_explain(etc_filename, *package, explain);
                else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
                        read_one_explain(usr_filename, *package, explain);
	}
	sort(explain.begin(), explain.end());
	explain.erase( unique( explain.begin(), explain.end() ), explain.end() );
	return 0;
}
