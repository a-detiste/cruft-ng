#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <sys/stat.h>
#include <cstdlib>

#include "mlocate_db.h"
//       original filename is 'db.h'
// TODO: should be packaged in /usr/include by 'mlocate' package
//       like 'make' provides '/usr/include/gnumake.h'
//       and  'sudo' provides '/usr/include/sudo_plugin.h'

//#define LIBDPKG_VOLATILE_API
//#include <dpkg/dpkg.h>
//#include <dpkg/dpkg-db.h>
//#include <dpkg/pkg-array.h>

using namespace std;

struct Cruft{
	std::string path;
	std::string owner;
	bool fs;
	bool db;
	bool explain;
	Cruft (std::string path)
	{
		this->path    = path;
		this->fs      = true;
		this->db      = false;
		this->explain = false;
	}
	Cruft (std::string path, std::string owner)
	{
		this->path    = path;
		this->owner   = owner;
		this->fs      = false;
		this->db      = true;
		this->explain = false;
	}
	Cruft (std::string path, std::string owner, bool fs, bool db, bool explain)
	{
		this->path    = path;
		this->owner   = owner;
		this->fs      = fs;
		this->db      = db;
		this->explain = explain;
	}
};

int main(int argc, char *argv[])
{
	system("updatedb");

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
	std::cout << "theorical length=" << header[10]*256 + header[11] << '\n'; // BAD !! UNSIGNED CHAR
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
	int size;
	size=cruft.size();
	std::cout << size << " files in cruft database\n\n";


	std::cout << "DPKG DATA\n";
	// TODO: read DPKG database directly instead of using dpkg-query
	FILE* fp;
	if ((fp = popen("dpkg-query --show --showformat '${binary:Package}\n'", "r")) == NULL) {
	    return 1;
	}
	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	std::vector<std::string> packages;
	std::string package;
	while (fgets(buf, sizeof(buf),fp))
	{
	      if (!(buf[0]=='u' and buf[1]=='n')) {
		package=buf;
		package=package.substr(0,package.size() - 1);
		//cout << package << endl;
		packages.push_back(package);
	      }
	}
	pclose(fp);
	int n_packages=packages.size();
	cout << n_packages << " packages installed"  << endl << endl;


	cout << "READING FILES IN DPKG DATABASE" << endl;
	// TODO: read DPKG database instead of using dpkg-query
        string command="dpkg-query --listfiles $(dpkg-query --show --showformat '${binary:Package} ')|sort -u";
	if ((fp = popen(command.c_str(), "r")) == NULL) {
	    return 1;
	}
	int progress=0;
	while (fgets(buf, sizeof(buf),fp))
	{
		progress++;
		if (progress % 1000 == 0) cout << progress << ' ' << std::flush;
		filename=buf;
		if (filename.substr(0,1)!="/") continue;
		filename=filename.substr(0,filename.size() - 1);
		//cout << packages[i] << ':' << filename << endl;

		// match against MLOCATE
		size=cruft.size(); // will be dynamicaly extended
		bool found=false;
		for (int j=1;j<size;j++) {
		      if (cruft[j].path==filename) {
			  cruft[j].db   =true;
			  found         =true;
			  break;
		      }
		}
		// TODO: ignore ${prunepaths} here also
		if (!found) cruft.push_back(Cruft(filename,"-"));
	}
        pclose(fp);
	std::cout << "done"  << endl << std::flush;

	// remove matched records (=most records)
	size=cruft.size();
	std::cout << size << " files in cruft database" << endl << std::flush;
	std::vector<Cruft> cruft2;
	while (!cruft.empty()) {
	    if ( !cruft.back().fs or !cruft.back().db)
	    {
		cruft2.push_back(Cruft(cruft.back().path,
				       cruft.back().owner,
				       cruft.back().fs,
				       cruft.back().db,
				       cruft.back().explain));
	    }
	    cruft.pop_back();
	}
	size=cruft2.size();
	std::cout << size << " files in cruft2 database" << endl << std::flush;


	// TODO: this should use DPKG database too
	for (int i=0;i<n_packages;i++) {
	      for (int j=0;j<size;j++) {
		  if (   cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".clilibs"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".conffiles"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".config"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".list"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".md5sums"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".postinst"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".postrm"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".preinst"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".prerm"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".shlibs"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".symbols"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".templates"
		      or cruft2[j].path=="/var/lib/dpkg/info/" + packages[i] + ".triggers") {
			      cruft2[j].explain=true;
			      cruft2[j].owner  =packages[i];
		  }
	      }
	}


	// match the globs against reduced database
	std::string filter_glob;
	std::string filter_file;
	struct stat stat_buffer;
	for (int i=0;i<n_packages;i++) {
		filter_file="/usr/lib/cruft/filters-unex/" + packages[i];
		if ( stat(filter_file.c_str(), &stat_buffer)==0 )
		{
			//cout << "filter for " << packages[i] << endl;
			std::ifstream filter_unex(filter_file.c_str());
			while (filter_unex.good())
			{
			      getline(filter_unex,filter_glob);
			      if (filter_unex.eof()) break;
			      if (filter_glob.substr(0,1) == "/") {
				    bool match=false;
				    for (int j=0;j<size;j++) {
					if ( cruft2[j].path==filter_glob ) {
					        cruft2[j].explain=true;
					        cruft2[j].owner  =packages[i];
						match            =true;
						break;
					} else if (filter_glob.find("**")==filter_glob.size()-2 ) {
						unsigned int length=filter_glob.size()-2;
						if (cruft2[j].path.size() >= length
					        and cruft2[j].path.substr(0,length)==filter_glob.substr(0,length)) {
						      cruft2[j].explain=true;
						      cruft2[j].owner  =packages[i] + "**";
						      match            =true;
						      // no break here !, because ** matches many files
						}
					} else if (filter_glob.find("*")==filter_glob.size()-1 ) {
						unsigned int length=filter_glob.size()-1;
						if (  cruft2[j].path.size() >= length
						  and cruft2[j].path.find("/",length)==string::npos
						  and cruft2[j].path.substr(0,length)==filter_glob.substr(0,length)) {
						      cruft2[j].explain=true;
						      cruft2[j].owner  =packages[i] + "*";
						      match            =true;
						}
					} else {
						// TODO: this only implement 1-1 match and path/starstar
						// no '?' or '*' or 'path/starstar/path'
						// use GLOB or LS ???
					}

				    }
				    //if (!match) cout << 'no match [' << filter_glob << ']' << endl;
			      }
			}
		}
	}

	// run needed scripts in /usr/lib/cruft/explain/ with popen()

	std::cout << "REPORT\n";
	size=cruft2.size();
	for (int i=0;i<size;i++) {
		if (   !cruft2[i].fs
		    or (!cruft2[i].db and !cruft2[i].explain)) {
			cout << cruft2[i].fs
			     << cruft2[i].db
			     << cruft2[i].explain
			     << ' '
			     << cruft2[i].owner
			     << ' '
			     << cruft2[i].path
			     << endl;
		}
	}

	return 0;
}
