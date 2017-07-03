#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
#include "dpkg.h"
#include "usr_merge.h"

int read_dpkg_header(vector<string>& packages)
{
	bool debug=getenv("DEBUG") != NULL;

	// TODO: read DPKG database directly instead of using dpkg-query
	if (debug) cerr << "DPKG DATA\n";
	FILE* fp;
	if ((fp = popen("dpkg-query --show --showformat '${binary:Package}\n'", "r")) == NULL) return 1;
	const int SIZEBUF = 4096;
	char buf[SIZEBUF];
	string package;
	while (fgets(buf, sizeof(buf),fp))
	{
		package=buf;
		package=package.substr(0,package.size() - 1); // remove '/n'
		//cerr << package << endl;
		packages.push_back(package);
	}
	pclose(fp);
	if (debug) cerr << packages.size() << " packages installed"  << endl << endl;
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

int read_diversions(vector<Diversion>& diversions)
{
	ifstream txt("/var/lib/dpkg/diversions");
	while(!txt.eof())
	{
		string oldfile,newfile,package;
		getline(txt,oldfile);
		getline(txt,newfile);
		getline(txt,package);
		diversions.push_back(Diversion(oldfile,newfile,package));
	}
	txt.close();
	return 0;
}

int read_dpkg_items(vector<string>& dpkg)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "READING FILES IN DPKG DATABASE" << endl;
	vector<Diversion> diversions;
	read_diversions(diversions);

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

		dpkg.push_back(usr_merge(filename));
	}
        pclose(fp);
	if (debug) cerr << "done"  << endl;
	sort(dpkg.begin(), dpkg.end()); // remove duplicates ???
	if (debug) cerr << dpkg.size() << " files in DPKG database" << endl;
	return 0;
}
