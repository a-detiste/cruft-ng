#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>

#include "plocate.h"

int read_plocate(vector<string>& fs, vector<string>& prunefs)
{
	bool debug=getenv("DEBUG") != NULL;

	string line;

	if (debug) cerr << "PLOCATE DATA\n";
	string command = "plocate .";
        const int SIZEBUF = 200;
        char buf[SIZEBUF];
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
		fs.push_back(filename);
	}
	pclose(fp);
	sort(fs.begin(), fs.end());
	if (debug) cerr << fs.size() << " relevant files in PLOCATE database"  << endl << endl;
	return 0;
}
