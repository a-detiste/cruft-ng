#include <iostream>
#include <algorithm>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include "explain.h"

static int upper(int c)
{
      // http://www.dreamincode.net/forums/topic/15095-convert-string-to-uppercase-in-c/
      return toupper((unsigned char)c);
}

int read_explain(vector<string>& packages, vector<string>& explain)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "EXECUTING FILTERS IN /usr/lib/cruft/explain/" << endl;

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
		if (package=="dpkg") continue; /* this is done in read_dpkg_items() */
		if (package=="USERS") continue; /* replaced by USERS_cruft-ng */
		bool match=false;
		string uppercase=package;
		transform(uppercase.begin(), uppercase.end(), uppercase.begin(), upper);
		if (package==uppercase)
			match=true;
		if (package=="USERS_cruft-ng")
			match=true;
		else {
			vector<string>::iterator it=packages.begin();
			for (;it !=packages.end();it++) {
			      if (package==*it) {
				    match=true;
				    break;
			      }
			}
		}
		if (debug) cerr << match << ' ' << package << endl;
		if (!match) continue;
		FILE* fp;
		if ((fp = popen(("/usr/lib/cruft/explain/" + package + " 3>&1").c_str(), "r")) == NULL) return 1;
		const int SIZEBUF = 200;
		char buf[SIZEBUF];
		string filter;
		while (fgets(buf, sizeof(buf),fp))
		{
			filter=buf;
			filter=filter.substr(0,filter.size() - 1); // remove '/n'
			//if (debug) cerr << "# " << filter << endl;
			explain.push_back(filter);
		}
		pclose(fp);
	}
	closedir(dp);
	sort(explain.begin(), explain.end());
	return 0;
}
