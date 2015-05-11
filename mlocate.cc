#include <iostream>
#include <fstream>
#include <algorithm>
#include "mlocate_db.h"
//       original filename is 'db.h'
// TODO: should be packaged in /usr/include by 'mlocate' package
//       like 'make' provides '/usr/include/gnumake.h'
//       and  'sudo' provides '/usr/include/sudo_plugin.h'

#include "mlocate.h"

int read_mlocate(vector<string>& fs, vector<string>& prunefs)
{
	bool debug=getenv("DEBUG") != NULL;

	string line;
	ifstream mlocate("/var/lib/mlocate/mlocate.db");
	if (not mlocate) {
		cerr << "can not open mlocate database" << endl << "are you root ?" << endl;
		// you can also "setgid mlocate" the cruft-ng binary
		exit(1);
 	}

	cerr << "MLOCATE HEADER:\n";
	//char * header = new char [sizeof(db_header)];
	struct db_header header;
	mlocate.read ((char *) &header,sizeof(db_header));
	// TODO: assert this

	for (int i=1;i<9;i++) {
		if (debug) cerr << header.magic[i];
	} if (debug) cerr << endl << endl;


	if (debug) cerr << "MLOCATE ROOT:" << endl;
	getline(mlocate,line, '\0');
	if (debug) cerr << line << endl << endl;

	if (debug) cerr << "MLOCATE PARAMETERS:" << endl;
	int param_start = mlocate.tellg();
	while (getline(mlocate,line,'\0'))
	{
		if (line.empty()) break;
		string key=line;
		if (debug) cerr << line << '=';
		while (getline(mlocate,line,'\0'))
		{
			if (line.empty()) break;
			if (key=="prunefs") prunefs.push_back(line);
			if (debug) cerr << line << ' ';
		}
		if (debug) cerr << endl;
	}
	// TODO "prunepaths="
	//  whitelist /tmp , /media
	//  ignore    paths that doesn't even exist
	//  warn      on other (e.g.: /var/spool)
	if (debug) cerr << "theorical length=" << header.conf_size << endl; // BAD !! UNSIGNED CHAR
	if (debug) cerr << "actual length=" << ((int) mlocate.tellg() - param_start - 1) << endl << endl;

	if (debug) cerr << "MLOCATE DATA\n";
	char * dir = new char [sizeof(db_directory)];
	string dirname;
	string filename;
	//fs.reserve(200000);
	while ( mlocate.good() ) {
		mlocate.read (dir,sizeof(db_directory));
		getline(mlocate,dirname,'\0');
		if (mlocate.eof()) break;
		char filetype; // =sizeof(db_entry)
		while ((filetype = mlocate.get()) != DBE_END) {
			getline(mlocate,filename,'\0');
			/* go into /home & /usr/local direct subfolders,
			 * but not deeper */
			if (dirname.length() >= 6  and dirname.substr(0, 6) == "/home/") continue;
		        //if (dirname.length() >= 11 and dirname.substr(0,11) == "/usr/local/") continue;
			string fullpath=dirname + '/' + filename;
			/* don't even go into /root */
			if (fullpath.length() >= 6  and fullpath.substr(0, 6) == "/root/") continue;
			/* spurious entry in mlocate db */
			if (fullpath.substr(0,28)=="/var/lib/mlocate/mlocate.db.") continue;
			//cerr << dirname << '/' << filename << endl;
			fs.push_back(fullpath);
		}
	}
	mlocate.close();
	sort(fs.begin(), fs.end());
	if (debug) cerr << prunefs.size() << " relevant records in PRUNEFS database" << endl;
	if (debug) cerr << fs.size() << " relevant files in MLOCATE database"  << endl << endl;
	return 0;
}
