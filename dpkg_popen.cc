#include <iostream>
#include <algorithm>
#include <stdio.h>
#include "dpkg.h"

#define debug false

int read_dpkg_header(vector<string>& packages)
{
	// TODO: read DPKG database directly instead of using dpkg-query
	if (debug) cout << "DPKG DATA\n";
	FILE* fp;
	if ((fp = popen("dpkg-query --show --showformat '${binary:Package}\n'", "r")) == NULL) return 1;
	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	string package;
	while (fgets(buf, sizeof(buf),fp))
	{
		package=buf;
		package=package.substr(0,package.size() - 1); // remove '/n'
		//cout << package << endl;
		packages.push_back(package);
	}
	pclose(fp);
	if (debug) cout << packages.size() << " packages installed"  << endl << endl;
	return 0;
}

int read_dpkg_items(vector<string>& dpkg)
{
	if (debug) cout << "READING FILES IN DPKG DATABASE" << endl;
	// TODO: read DPKG database instead of using dpkg-query
	// cat /var/lib/dpkg/info/ *.list |sort -u
        string command="dpkg-query --listfiles $(dpkg-query --show --showformat '${binary:Package} ')|sort -u";
	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	FILE* fp;
	if ((fp = popen(command.c_str(), "r")) == NULL) return 1;
	while (fgets(buf, sizeof(buf),fp))
	{
		string filename=buf;
		if (filename.substr(0,1)!="/") continue;
		filename=filename.substr(0,filename.size() - 1);
		// TODO: ignore ${prunepaths} here also
		dpkg.push_back(filename);
	}
        pclose(fp);
	if (debug) cout << "done"  << endl;
	sort(dpkg.begin(), dpkg.end()); // remove duplicates ???
	if (debug) cout << dpkg.size() << " files in DPKG database" << endl;
	return 0;
}
