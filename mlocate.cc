// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include "mlocate_db.h"
//       original filename is 'db.h'

#include "locate.h"
#include "python.h"

#include <experimental/filesystem>
using namespace std::experimental;
namespace fs = std::experimental::filesystem;

int read_locate(vector<string>& fs, const string& ignore_path) // vector<string>& prunefs
{
	bool debug=getenv("DEBUG") != NULL;

	init_python();

	string line;
	ifstream mlocate("/var/lib/mlocate/mlocate.db");
	if (not mlocate) {
		cerr << "can not open mlocate database" << endl << "are you root ?" << endl;
		// you can also "setgid mlocate" the cruft-ng binary
		exit(1);
 	}

	if (debug) cerr << "MLOCATE HEADER:\n";
	//char * header = new char [sizeof(db_header)];
	struct db_header header;
	mlocate.read ((char *) &header,sizeof(db_header));
	// TODO: assert this

	for (int i=0;i<8;i++) {
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
			//if (key=="prunefs") prunefs.push_back(line);
			if (debug) cerr << line << ' ';
		}
		if (debug) cerr << endl;
	}
	// TODO "prunepaths="
	//  whitelist /tmp , /media
	//  ignore    paths that doesn't even exist
	//  warn      on other (e.g.: /var/spool)
	if (debug) cerr << "theoretical length=" << header.conf_size << endl; // BAD !! UNSIGNED CHAR
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
		string toplevel = dirname.substr(0, dirname.find('/', 1));
		if (   toplevel == "/dev"
		    /* have a peek into /home, but not deeper */
		    or (toplevel == "/home" and dirname != "/home")
		    or toplevel == "/mnt"
		    or toplevel == "/root"
		    or toplevel == "/tmp"
		    or dirname == "/var/lib/dpkg/info")
		while ((mlocate.get()) != DBE_END) {
			mlocate.ignore(std::numeric_limits<int>::max(), '\0');
		} else while ((filetype = mlocate.get()) != DBE_END) {
			getline(mlocate,filename,'\0');
			string fullpath=dirname + '/' + filename;
			if (!pyc_has_py(fullpath, debug))
				fs.emplace_back(fullpath);
		}
	}
	mlocate.close();

	// default PRUNEPATH in /etc/updatedb.conf
	fs.emplace_back("/var/spool");
	try {
		for (const auto& entry: filesystem::recursive_directory_iterator{"/var/spool", filesystem::directory_options::skip_permission_denied})
		{
			fs.emplace_back(entry.path());
		}
	} catch(const exception& e) {
		cerr << "Failed to iterate directory /var/spool/: " << e.what() << endl;
	}

        sort(fs.begin(), fs.end());
        fs.erase( unique( fs.begin(), fs.end() ), fs.end() );

	//if (debug) cerr << prunefs.size() << " relevant records in PRUNEFS database" << endl;
	if (debug) cerr << fs.size() << " relevant files in MLOCATE database"  << endl << endl;
	return 0;
}
