#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include "explain.h"
#include "usr_merge.h"

void read_one_explain(const string& script,vector<string>& explain)
{
	//bool debug=getenv("DEBUG") != NULL;

	FILE* fp;
	if ((fp = popen((script).c_str(), "r")) == NULL) return;
	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	string filter;
	while (fgets(buf, sizeof(buf),fp))
	{
		filter=buf;
		filter=filter.substr(0,filter.size() - 1); // remove '/n'
		//if (debug) cerr << "# " << filter << endl;
		explain.push_back(usr_merge(filter));
	}
	pclose(fp);
}

void read_uppercase(vector<string>& explain, string directory)
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
			read_one_explain(directory + package, explain);
	}
	closedir(dp);
	if (debug) cerr << endl;
}

int read_explain(vector<string>& packages, vector<string>& explain)
{
	bool debug=getenv("DEBUG") != NULL;

	read_uppercase(explain, "/usr/libexec/cruft/");
	read_uppercase(explain, "/etc/cruft/explain/");

	if (debug) cerr << "EXECUTING OTHER FILTERS" << endl;
	vector<string>::iterator package;
	for (package=packages.begin();package !=packages.end();package++) {
                struct stat stat_buffer;
                string etc_filename = "/etc/cruft/explain/" + *package;
                string usr_filename = "/usr/libexec/cruft/" + *package;
                if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
                        read_one_explain(etc_filename, explain);
                else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
                        read_one_explain(usr_filename, explain);
	}
	sort(explain.begin(), explain.end());
	explain.erase( unique( explain.begin(), explain.end() ), explain.end() );
	return 0;
}
