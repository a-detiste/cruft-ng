#include <iostream>
#include <algorithm>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include "explain.h"
#include "usr_merge.h"

void read_one_explain(const string& script,vector<string>& explain)
{
	//bool debug=getenv("DEBUG") != NULL;

	FILE* fp;
	if ((fp = popen((script + " 3>&1").c_str(), "r")) == NULL) return;
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

int read_explain(vector<string>& packages, vector<string>& explain)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "EXECUTING UPPERCASE FILTERS IN /usr/lib/cruft/explain/" << endl;
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/usr/lib/cruft/explain/")) == NULL) {
	      cerr << "Error(" << errno << ") opening /usr/lib/cruft/explain/" << endl;
	      return errno;
	}
	while ((dirp = readdir(dp)) != NULL) {
		string package=string(dirp->d_name);
		if (package==".") continue;
		if (package=="..") continue;
		if (package=="USERS") continue; // replaced by USERS_cruft-ng
		string uppercase=package;
		transform(uppercase.begin(), uppercase.end(), uppercase.begin(), ::toupper);
		if (package==uppercase or package=="USERS_cruft-ng")
			read_one_explain("/usr/lib/cruft/explain/" + package, explain);
	}
	if (debug) cerr << endl;

	if (debug) cerr << "EXECUTING UPPERCASE FILTERS IN /etc/cruft/explain/" << endl;
	if((dp = opendir("/etc/cruft/explain/")) == NULL) {
	      cerr << "Error(" << errno << ") opening /etc/cruft/explain/" << endl;
	      return errno;
	}
	while ((dirp = readdir(dp)) != NULL) {
		string package=string(dirp->d_name);
		if (package==".") continue;
		if (package=="..") continue;
		string uppercase=package;
		transform(uppercase.begin(), uppercase.end(), uppercase.begin(), ::toupper);
		if (package==uppercase)
			read_one_explain("/etc/cruft/explain/" + package, explain);
	}
	if (debug) cerr << endl;


	if (debug) cerr << "EXECUTING OTHER FILTERS" << endl;
	vector<string>::iterator it=packages.begin();

	string retain;
	for (;it !=packages.end();it++) {
                string package=*it;
                size_t arch=package.find(":");
                if (arch != string::npos ) package=package.substr(0,arch);
                if (package==retain) continue;
                retain=package;

		if (package=="dpkg") continue; /* this is done in read_dpkg_items() */

                struct stat stat_buffer;
                string etc_filename = "/etc/cruft/explain/" + package;
                string usr_filename = "/usr/lib/cruft/explain/" + package;
                if ( stat(etc_filename.c_str(), &stat_buffer)==0 )
                        read_one_explain(etc_filename, explain);
                else if ( stat(usr_filename.c_str(), &stat_buffer)==0 )
                        read_one_explain(usr_filename, explain);
	}
	sort(explain.begin(), explain.end());
	explain.erase( unique( explain.begin(), explain.end() ), explain.end() );
	return 0;
}
