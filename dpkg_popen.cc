// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "dpkg.h"
#include "usr_merge.h"

void dpkg_start() {}
void dpkg_end() {}

int query(const char *path)
{
	// not implemented: diversions
	FILE* fp;
	setenv("LANG", "C", 1);
	char buf[4000];
	char *pos;

	sprintf(buf, "dpkg-query --search '%s' 2>/dev/null", path);
	if ((fp = popen(buf, "r")) == NULL) return 0;
	if (!fgets(buf, sizeof(buf), fp)) return 0;
	pos = strchr(buf, ':');
	pos[0] = '\0';
	pos = strchr(buf, ',');
	if(pos) pos[0] = '\0';
	puts(buf);
	return 1;
}

int read_dpkg_header(vector<string>& packages)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "DPKG DATA\n";
	FILE* fp;
	if ((fp = popen("dpkg-query --show --showformat '${Package}\n' | sort -u", "r")) == NULL) return 1;
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

int read_diversions(vector<Diversion>& diversions)
{
	bool debug=getenv("DEBUG") != NULL;

	FILE* fp;
        setenv("LANG", "C", 1);
	if ((fp = popen("dpkg-divert --list", "r")) == NULL) return 1;

	const int SIZEBUF = 4096;
	char buf[SIZEBUF];
	while (fgets(buf, sizeof(buf),fp))
	{
		const char* delim = " ";
		bool local;
		const char* LOCAL = "local";
		string oldfile,newfile,package;

		if (debug) cerr << buf << endl;
		//diversion of /usr/share/dict/words to /usr/share/dict/words.pre-dictionaries-common by dictionaries-common
		//diversion of /usr/share/man/man1/sh.1.gz to /usr/share/man/man1/sh.distrib.1.gz by dash
		//diversion of /usr/bin/firefox to /usr/bin/firefox.real by firefox-esr
		//diversion of /bin/sh to /bin/sh.distrib by dash
		//local diversion of /etc/apt/apt.conf.d/20packagekit to /etc/PackageKit/20packagekit.distrib

		// bug #1010362
		local = !strncmp(strtok((char*)buf, delim), LOCAL, strlen(LOCAL));
		if (local) {
			strtok(NULL, delim);
		}

		strtok(NULL, delim);

		oldfile = strtok(NULL, delim);

		strtok(NULL, delim);

		newfile = strtok(NULL, delim);

		if (local) {
			newfile = newfile.substr(0,newfile.size() - 1); // remove '/n'
			package = LOCAL;
		} else {
			strtok(NULL, delim);
			package = strtok(NULL, delim);
			package = package.substr(0,package.size() - 1); // remove '/n'
		}

		diversions.push_back(Diversion(oldfile,newfile,package));
	}
	pclose(fp);
	if (debug) cerr << diversions.size() << " files diverted"  << endl << endl;

	return 0;
}

static int read_dpkg_items(vector<string>& dpkg)
{
	bool debug=getenv("DEBUG") != NULL;

	if (debug) cerr << "READING FILES IN DPKG DATABASE" << endl;
	vector<Diversion> diversions;
	read_diversions(diversions);

	dpkg.push_back("/");

        string command="dpkg-query --listfiles $(dpkg-query --show --showformat '${binary:Package} ')|sort -u";
	const int SIZEBUF = 300;
	char buf[SIZEBUF];
	FILE* fp;
	if ((fp = popen(command.c_str(), "r")) == NULL) return 1;
	while (fgets(buf, sizeof(buf),fp))
	{
		string filename=buf;
		if (filename.substr(0,1)!="/") continue;
		filename=filename.substr(0,filename.size() - 1);
		vector<Diversion>::iterator it=diversions.begin();
		struct stat stat_buffer;
		for(;it !=diversions.end();it++) {
			if (filename==(*it).oldfile
		            && stat(filename.c_str(),&stat_buffer)!= 0) filename=(*it).newfile;
		}

		dpkg.push_back(usr_merge(filename));


		// also consider all intermediate subdirectories under /etc
		if (filename.substr(0,5)!="/etc/")
			continue;

		if (stat(filename.c_str(),&stat_buffer) == 0)
			if ((stat_buffer.st_mode & S_IFDIR) != 0)
				continue;

		while (1)
		{
			size_t found;
			found=filename.find_last_of("/");
			filename=filename.substr(0,found);
			if (filename == "/etc")
				break;

			dpkg.push_back(filename);
		}
	}
        pclose(fp);
	if (debug) cerr << "done"  << endl;
	sort(dpkg.begin(), dpkg.end());
	// remove duplicates
	dpkg.erase( unique( dpkg.begin(), dpkg.end() ), dpkg.end() );
	if (debug) cerr << dpkg.size() << " files in DPKG database" << endl;
	return 0;
}

int read_dpkg(vector<string>& packages, vector<string>& files, bool csv) {
	read_dpkg_header(packages);
	read_dpkg_items(files);

	// some more horrible hack for Buster backport
	if (csv) (system("/usr/lib/cruft/dpkg_csv.py")==0);

	return 0;
}
