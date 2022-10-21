#include <cstring>
#include <iostream>
#include <algorithm>

#include "plocate.h"
#include "python.h"

// build fail on hurd-i386
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int read_plocate(vector<string>& fs, vector<string>& prunefs)
{
	bool debug=getenv("DEBUG") != NULL;

	string line;

	if (debug) cerr << "PLOCATE DATA\n";

	fs.push_back("/.");
	fs.push_back("/dev");
	fs.push_back("/home");
	fs.push_back("/root");
	fs.push_back("/tmp");

	string command = "plocate /";
	char buf[PATH_MAX];
	FILE* fp;
	if ((fp = popen(command.c_str(), "r")) == NULL) return 1;
	while (fgets(buf, sizeof(buf),fp))
	{
		buf[strlen(buf)-1] = '\0';
		string filename = buf;
		string toplevel = filename.substr(0, filename.find("/", 1));
		if (   toplevel == "/dev"
		    or (toplevel == "/home" /* and dirname != "/home" */)
		    or toplevel == "/mnt"
		    or toplevel == "/root"
		    or toplevel == "/tmp")
			continue;
		if (!pyc_has_py(filename, false)) fs.push_back(filename);
	}
	pclose(fp);

	// default PRUNEPATH in /etc/updatedb.conf
	command = "find /var/spool 2> /dev/null";
	if ((fp = popen(command.c_str(), "r")) == NULL) return 1;
	while (fgets(buf, sizeof(buf),fp))
	{
		buf[strlen(buf)-1] = '\0';
		string filename = buf;
		fs.push_back(filename);
	}
	pclose(fp);

	sort(fs.begin(), fs.end());
	fs.erase( unique( fs.begin(), fs.end() ), fs.end() );
	if (debug) cerr << fs.size() << " relevant files in PLOCATE database"  << endl << endl;
	return 0;
}
