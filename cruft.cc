#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
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

int read_mlocate(vector<string>& fs)
{
	string line;
	ifstream mlocate("/var/lib/mlocate/mlocate.db");
	if (not mlocate) {
		cout << "can not open mlocate database" << endl << "are you root ?" << endl;
		// you can alos "setgid mlocate" the cruft-ng binary
		return 1;
 	}

	//std::cout << "MLOCATE HEADER\n";
	char * header = new char [sizeof(db_header)];
	mlocate.read (header,sizeof(db_header));
	// TODO: assert this
	/*
	for (int i=0;i<sizeof(db_header);i++) {
		int n=header[i];
		cout << i << ':' <<
			header[i] <<
			'(' <<
			n <<
		        ")\n";
	}
	std::cout << '\n';
	*/

	cout << "MLOCATE LOCATION\n";
	getline(mlocate,line, '\0');
	cout << line << "\n\n";

	cout << "MLOCATE PARAMETERS\n";
	int param_start = mlocate.tellg();
	while (getline(mlocate,line,'\0'))
	{
		if (line.empty()) break;
		cout << line << '=';
		while (getline(mlocate,line,'\0'))
		{
			if (line.empty()) break;
			cout << line << ' ';
		}
		cout << endl;
	}
	// TODO "prunepaths="
	//  whitelist /tmp , /media
	//  ignore    paths that doesn't even exist
	//  warn      on other (e.g.: /var/spool)
	cout << "theorical length=" << header[10]*256 + header[11] << '\n'; // BAD !! UNSIGNED CHAR
	cout << "actual length=" << int(mlocate.tellg()) - param_start - 1 << "\n\n";

	cout << "MLOCATE DATA\n";
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
			if (dirname.length() >= 5  and dirname.substr(0, 5) == "/home") continue;
			if (dirname.length() >= 5  and dirname.substr(0, 5) == "/root") continue;
		        if (dirname.length() >= 10 and dirname.substr(0,10) == "/usr/local") continue;
			//std::cout << dirname << '/' << filename << endl;
			fs.push_back(dirname + '/' + filename);
		}
	}
	mlocate.close();
	sort(fs.begin(), fs.end());
	cout << fs.size() << " relevant files in MLOCATE database\n\n";

	return 0;
}

int read_dpkg_header(vector<string>& packages)
{
	// TODO: read DPKG database directly instead of using dpkg-query
	cout << "DPKG DATA\n";
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
	cout << packages.size() << " packages installed"  << endl << endl;
	return 0;
}

int read_dpkg_items(vector<string>& dpkg)
{
	cout << "READING FILES IN DPKG DATABASE" << endl;
	// TODO: read DPKG database instead of using dpkg-query
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
	cout << "done"  << endl;
	sort(dpkg.begin(), dpkg.end()); // remove duplicates ???
	cout << dpkg.size() << " files in DPKG database" << endl;
	return 0;
}

int read_globs(/* const */ vector<string>& packages, vector<string>& globs)
{
	cout << "READING GLOBS IN /usr/lib/cruft/filters-unex/" << endl;
	vector<string>::iterator it=packages.begin();
	while (it !=packages.end()) {
		string package=*it;
		unsigned int arch=package.find(":");
		if (arch != string::npos ) package=package.substr(0,arch);
		// BUG: if libc6:i386 & libc6:amd64 are installed,
		// the globs are read twice

		struct stat stat_buffer;
		string glob_filename ="/usr/lib/cruft/filters-unex/" + package;
		if ( stat(glob_filename.c_str(), &stat_buffer)==0 )
		{
			ifstream glob_file(glob_filename.c_str());
			while (glob_file.good())
			{
				string glob_line;
				getline(glob_file,glob_line);
				if (glob_file.eof()) break;
				if (glob_line.substr(0,1) == "/") globs.push_back(glob_line);
			}
			glob_file.close();
		}
		it++;
	}
	sort(globs.begin(), globs.end());
	cout << globs.size() << " globs in database" << endl << endl;
	// !!! TODO: remove duplicates
	return 0;
}

void updatedb()
{
	//TODO: compare mtime /var/cache/apt/pkgcache.bin
	//      et mtime      /var/lib/mlocate/mlocate.db
	//      et date systeme
	system("updatedb");
}

int main(int argc, char *argv[])
{
	std::vector<string> packages;
	read_dpkg_header(packages);

	std::vector<string> globs;
	read_globs(packages,globs);

	updatedb();

	std::vector<string> fs;
	read_mlocate(fs);

	std::vector<string> dpkg;
	read_dpkg_items(dpkg);

	// match two main data sources
	vector<string> cruft;
	vector<string> missing;
	vector<string>::iterator left=fs.begin();
	vector<string>::iterator right=dpkg.begin();
	while (left != fs.end() && right != dpkg.end() )
	{
		//cout << "[" << *left << "=" << *right << "]" << endl;
		if (*left==*right) {
			left++;
			right++;
		} else if (*left < *right) {
			cruft.push_back(*left);
			left++;
		} else {
			missing.push_back(*right);
			right++;
		}
	}
	//fs.clear();
	//dpkg.clear();

	cout << endl << missing.size() << " files in missing database" << endl;
	cout << cruft.size() << " files in cruft database" << endl << endl << flush;

	// TODO: this should use DPKG database too
	vector<string> cruft2;
	left=cruft.begin();
	right=packages.begin();
	while (left != cruft.end() && right != packages.end() ) {
		// TODO: replace with substring or something else
		if (*left < "/var/lib/dpkg/info") {
			cruft2.push_back(*left);
			left++;
		} else if (*left > "/var/lib/dpkg/info/zzzzz" ) {
			cruft2.push_back(*left);
			left++;
		} else {
			right=packages.begin();
			bool found=false;
			while(right != packages.end() ) {
			    if ( *left=="/var/lib/dpkg/info/" + *right + ".clilibs"
			      or *left=="/var/lib/dpkg/info/" + *right + ".conffiles"
			      or *left=="/var/lib/dpkg/info/" + *right + ".config"
			      or *left=="/var/lib/dpkg/info/" + *right + ".list"
			      or *left=="/var/lib/dpkg/info/" + *right + ".md5sums"
			      or *left=="/var/lib/dpkg/info/" + *right + ".postinst"
			      or *left=="/var/lib/dpkg/info/" + *right + ".postrm"
			      or *left=="/var/lib/dpkg/info/" + *right + ".preinst"
			      or *left=="/var/lib/dpkg/info/" + *right + ".prerm"
			      or *left=="/var/lib/dpkg/info/" + *right + ".shlibs"
			      or *left=="/var/lib/dpkg/info/" + *right + ".symbols"
			      or *left=="/var/lib/dpkg/info/" + *right + ".templates"
			      or *left=="/var/lib/dpkg/info/" + *right + ".triggers")
				{ found=true; break; }
				right++;
			}
			if (!found) cruft2.push_back(*left);
			left++;
		}
	}
	//cruft.clear();
	std::cout << cruft2.size() << " files in cruft2 database" << endl << endl << std::flush;

	// match the globs against reduced database
	vector<string> cruft3;
	left=cruft2.begin();
	while (left != cruft2.end()) {
		right=globs.begin();
		bool match=false;
		while (right != globs.end()) {
			if (*left==*right) {
				match=true;
				//cout << "1/1 MATCH: " << *left << endl;
				break;
			} else if ( (*right).find("**")==(*right).size()-2 ) {
				unsigned int length=(*right).size()-2;
				if (    (*left).size() >= length
				    and (*left).substr(0,length)==(*right).substr(0,length)) {
					match=true;
					//cout << "** MATCH: " << *left << " = " << *right << endl;
					break;
				}
			} else if ( (*right).find("*")==(*right).size()-1 ) {
				unsigned int length=(*right).size()-1;
				if (  (*left).size() >= length
				  and (*left).find("/",length)==string::npos
				  and (*left).substr(0,length)==(*right).substr(0,length)) {
					match=true;
					//cout << "* MATCH: " << *left << " = " << *right << endl;
					break;
				}
			} else {
				// TODO: this only implement 1-1 match, path/star and path/starstar
				// no '?' or '*' or 'path/starstar/path'
				// use GLOB or LS ???
			}
			right++;
		}
		if (!match) cruft3.push_back(*left);
		left++;
	}

	//cruft2.clear();
	cout << cruft3.size() << " files in cruft3 database" << endl << endl << flush;

	// run needed scripts in /usr/lib/cruft/explain/ with popen()
	cout << "cruft report: lundi 22 septembre 2014, 17:42:27 (UTC+0200)" << endl << endl;

	cout << "---- missing: dpkg ----" << endl;
	for (int i=0;i<missing.size();i++) {
		cout << "        " << missing[i] << endl;
	}

	//TODO: split by filesystem
	cout << "---- unexplained: / ----" << endl;
	for (int i=0;i<cruft3.size();i++) {
		cout << "        " << cruft3[i] << endl;
	}

	// NOT IMPLEMENTED
	cout << "---- broken symlinks: / ----" << endl;
	cout << endl << "end." << endl;
	return 0;
}
