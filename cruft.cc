#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <string.h>
#include "mlocate.h"

extern int shellexp(char* filename, char* pattern );

//       original filename is 'db.h'
// TODO: should be packaged in /usr/include by 'mlocate' package
//       like 'make' provides '/usr/include/gnumake.h'
//       and  'sudo' provides '/usr/include/sudo_plugin.h'

//#define LIBDPKG_VOLATILE_API
//#include <dpkg/dpkg.h>
//#include <dpkg/dpkg-db.h>
//#include <dpkg/pkg-array.h>
using namespace std;

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

	string retain;
	for (;it !=packages.end();it++) {
		string package=*it;
		int arch=package.find(":");
		if (arch != string::npos ) package=package.substr(0,arch);
		if (package==retain) continue;
		retain=package;

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

bool myglob(string file, string glob )
{
	if (file==glob) return true;
	int filesize=file.size();
	int globsize=glob.size();
		if ( glob.find("**")==globsize-2 
		  and filesize >= globsize-2
		  and file.substr(0,globsize-2)==glob.substr(0,globsize-2)) {
		//cout << "match ** " << file << " # " << glob << endl;
		return true;
	}  else if ( glob.find("*")==globsize-1 
		  and filesize >= globsize-1
		  and file.find("/",globsize-1)==string::npos
		  and file.substr(0,globsize-1)==glob.substr(0,globsize-1)) {
		//cout << "match * " << file << " # " << glob << endl;
		return true;
	} else if ( fnmatch(glob.c_str(),file.c_str(),FNM_PATHNAME)==0 ) {
		//cout << "fnmatch " << file << " # " << glob << endl;	  
		return true;
	} else {
		// fallback to shellexp.c
		char param1[128];
		strcpy(param1,file.c_str());
		char param2[128];
		strcpy(param2,glob.c_str());
		bool result=shellexp(param1,param2);
		//cout << result << ' ' << file << " # " << glob << endl;
		return result;
	}
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
			match=myglob(*left,*right);
			if (match) break;
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
