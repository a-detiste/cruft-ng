#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "mlocate_db.h"
//       original filename is 'db.h'
// TODO: should be packaged in /usr/include by 'mlocate' package
//       like 'make' provides '/usr/include/gnumake.h'
//       and  'sudo' provides '/usr/include/sudo_plugin.h'

#include <cstdlib>

#define LIBDPKG_VOLATILE_API
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
//#include <dpkg/pkg-array.h>

using namespace std;

struct Cruft{
	std::string path;
	bool fs;
	bool db;
	Cruft (std::string path)
	{
		this->path = path;
		this->fs = true;
		this->db = false;
	}
};

int main(int argc, char *argv[])
{
	std::string line;
	std::ifstream mlocate("/var/lib/mlocate/mlocate.db");
	if (not mlocate) {
		std::cout << "can not open mlocate database\nare you root ?\n";
		return 1;
 	}

	//std::cout << "MLOCATE HEADER\n";
	char * header = new char [sizeof(db_header)];
	mlocate.read (header,sizeof(db_header));
	// TODO: assert this
	/*
	for (int i=0;i<sizeof(db_header);i++) {
		int n=header[i];
		std::cout << i << ':' <<
			header[i] <<
			'(' <<
			n <<
		        ")\n";
	}
	std::cout << '\n';
	*/

	std::cout << "MLOCATE LOCATION\n";
	getline(mlocate,line, '\0');
	std::cout << line << "\n\n";

	std::cout << "MLOCATE PARAMETERS\n";
	int param_start = mlocate.tellg();
	while (getline(mlocate,line,'\0'))
	{
		if (line.empty()) break;
		std::cout << line << '=';
		while (getline(mlocate,line,'\0'))
		{
			if (line.empty()) break;
			std::cout << line << ' ';
		}
		std::cout << endl;
	}
	// TODO "prunepaths="
	//  whitelist /tmp , /media
	//  ignore    paths that doesn't even exist
	//  warn      on other (e.g.: /var/spool)
	std::cout << "theorical length=" << header[10]*256 + header[11] << '\n'; // BAD
	std::cout << "actual length=" << int(mlocate.tellg()) - param_start - 1 << "\n\n";

	std::cout << "MLOCATE DATA\n";
	char * dir = new char [sizeof(db_directory)];
	std::string dirname;
	std::string filename;
	std::vector<Cruft> cruft;
	cruft.reserve(200000);
	while ( mlocate.good() ) {
		mlocate.read (dir,sizeof(db_directory));
		getline(mlocate,dirname,'\0');
		if (mlocate.eof()) break;
		char filetype; // =sizeof(db_entry)
		while ((filetype = mlocate.get()) != DBE_END) {;
			getline(mlocate,filename,'\0');
			if (dirname.length() >= 5  and dirname.substr(0, 5) == "/home") continue;
			if (dirname.length() >= 5  and dirname.substr(0, 5) == "/root") continue;
		        if (dirname.length() >= 10 and dirname.substr(0,10) == "/usr/local") continue;
			//std::cout << dirname << '/' << filename << endl;
			cruft.push_back(Cruft(dirname + '/' + filename));
		}
	}
	mlocate.close();

	std::cout << "DPKG DATA\n\n";
	// TODO: read DPKG database instead of using dpkg-query

	std::cout << "REPORT\n";
	int size=cruft.size();
	for (int i=0;i<size;i++) {
		filename=cruft.at(i).path;
		int rc=system(("/usr/bin/dpkg-query -S " + filename + " 2> /dev/null > /dev/null").c_str()); // BAD
		if (rc>0) {
			cout << "cruft:" << filename << endl;
		}
	}
	return 0;
}
