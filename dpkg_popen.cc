#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
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

struct Diversion{
	string oldfile;
	string newfile;
	string package;
	Diversion(string oldfile,string newfile,string package)
	{
		this->oldfile=oldfile;
		this->newfile=newfile;
		this->package=package;
	}
};


int read_dpkg_items(vector<string>& dpkg)
{
	if (debug) cout << "READING FILES IN DPKG DATABASE" << endl;
	vector<Diversion> diversions;
	ifstream diversion("/var/lib/dpkg/diversions");
	while(!diversion.eof())
	{
		string oldfile,newfile,package;
		getline(diversion,oldfile);
		getline(diversion,newfile);
		getline(diversion,package);
		diversions.push_back(Diversion(oldfile,newfile,package));
	}

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
		vector<Diversion>::iterator it=diversions.begin();
		struct stat stat_buffer;
		for(;it !=diversions.end();it++) {
			if (filename==(*it).oldfile
		            && stat(filename.c_str(),&stat_buffer)!= 0) filename=(*it).newfile;
		}

		dpkg.push_back(filename);
	}
        pclose(fp);
	if (debug) cout << "done"  << endl;
	sort(dpkg.begin(), dpkg.end()); // remove duplicates ???
	if (debug) cout << dpkg.size() << " files in DPKG database" << endl;
	return 0;
}
