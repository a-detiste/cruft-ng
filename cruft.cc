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
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "mlocate.h"
#include "dpkg.h"

bool debug=false;

extern int shellexp(char* filename, char* pattern );

using namespace std;

int read_globs(/* const */ vector<string>& packages, vector<string>& globs)
{
	if (debug) cout << "READING GLOBS IN /usr/lib/cruft/filters-unex/" << endl;
	vector<string>::iterator it=packages.begin();

	string retain;
	for (;it !=packages.end();it++) {
		string package=*it;
		int arch=package.find(":");
		if (arch != string::npos ) package=package.substr(0,arch);
		if (package==retain) continue;
		retain=package;

		struct stat stat_buffer;
		string glob_filename;
		if ( stat(("/etc/cruft/filters/" + package).c_str(), &stat_buffer)==0 )
			glob_filename="/etc/cruft/filters/" + package;
		else if ( stat(("/usr/lib/cruft/filters-unex/" + package).c_str(), &stat_buffer)==0 )
			glob_filename ="/usr/lib/cruft/filters-unex/" + package;
		else continue;

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
	sort(globs.begin(), globs.end());
	if (debug) cout << globs.size() << " globs in database" << endl << endl;
	// !!! TODO: remove duplicates
	return 0;
}

int upper(int c)
{
      // http://www.dreamincode.net/forums/topic/15095-convert-string-to-uppercase-in-c/
      return std::toupper((unsigned char)c);
}


int execute_filters(vector<string>& packages, vector<string>& filters)
{
	if (debug) cout << "EXECUTING FILTERS IN /usr/lib/cruft/explain/" << endl;

	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/usr/lib/cruft/explain/")) == NULL) {
	      cout << "Error(" << errno << ") opening /usr/lib/cruft/explain/" << endl;
	      return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		string package=string(dirp->d_name);
		if (package==".") continue;
		if (package=="..") continue;
		if (package=="dpkg") continue; /* this is done in read_dpkg_items() */
		bool match=false;
		string uppercase=package;
		transform(uppercase.begin(), uppercase.end(), uppercase.begin(), upper);
		if (package==uppercase)
			match=true;
		else {
			vector<string>::iterator it=packages.begin();
			for (;it !=packages.end();it++) {
			      if (package==*it) {
				    match=true;
				    break;
			      }
			}
		}
		if (debug) cout << match << ' ' << package << endl;
		if (!match) continue;
		FILE* fp;
		if ((fp = popen(("/usr/lib/cruft/explain/" + package + " 3>&1").c_str(), "r")) == NULL) return 1;
		const int SIZEBUF = 200;
		char buf[SIZEBUF];
		string filter;
		while (fgets(buf, sizeof(buf),fp))
		{
			filter=buf;
			filter=filter.substr(0,filter.size() - 1); // remove '/n'
			if (debug) cout << "# " << filter << endl;
			filters.push_back(filter);
		}
		pclose(fp);
	}
	closedir(dp);
	sort(filters.begin(), filters.end());
	return 0;
}

int read_mounts(vector<string>& prunefs, vector<string>& mounts)
{
	// this doesn't include "/", as it allways exists
	ifstream mtab("/proc/mounts");
	string mount,type;
	while ( not mtab.eof() )
	{
	      getline(mtab,mount,' '); // discard device
	      getline(mtab,mount,' ');
	      getline(mtab,type,' ');
	      vector<string>::iterator it=prunefs.begin();
	      bool match=false;
		   if (mount=="/") match=true;
	      else if (mount.substr(0,5)=="/sys/") match=true;
	      else if (mount.substr(0,5)=="/dev/") match=true;
	      else for (;it !=prunefs.end();it++) {
		      string uppercase;
		      uppercase=type;
		      transform(uppercase.begin(), uppercase.end(), uppercase.begin(), upper);
		      if (uppercase==*it) {
			  match=true;
			  break;
		      }
	      }
	      if (!match) {
		     //cout << mount << " => " << type << endl;
		     mounts.push_back(mount);
	      }
	      getline(mtab,mount); // discard options
	}
	return 0;
}

void updatedb()
{
	//TODO: compare mtime /var/cache/apt/pkgcache.bin
	//      et mtime      /var/lib/mlocate/mlocate.db
	//      et date systeme
        if (!getuid()) system("updatedb");
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
	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	FILE* fp;
	if ((fp = popen("date", "r")) == NULL) return 1;
	fgets(buf, sizeof(buf),fp);
	fclose(fp);
	cout << "cruft report: " << buf << endl << flush;

	std::vector<string> packages;
	read_dpkg_header(packages);

	std::vector<string> explain;
	execute_filters(packages,explain);

	std::vector<string> globs;
	read_globs(packages,globs);

	updatedb();

	std::vector<string> fs,prunefs,mounts;
	read_mlocate(fs,prunefs);
	read_mounts(prunefs,mounts);

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
			// file may exist on tmpfs
			// e.g.: /var/cache/apt/archives/partial
			struct stat stat_buffer;
	                if ( stat((*right).c_str(), &stat_buffer)!=0 && *right != "/.") missing.push_back(*right);
			right++;
		}
	}
	//fs.clear();
	//dpkg.clear();

	if (debug) cout << endl << missing.size() << " files in missing database" << endl;
	if (debug) cout << cruft.size() << " files in cruft database" << endl << endl << flush;

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
	if (debug) std::cout << cruft2.size() << " files in cruft2 database" << endl << endl << std::flush;

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
	if (debug) cout << cruft3.size() << " files in cruft3 database" << endl << endl << flush;

	// match the dynamic "explain" filters
	vector<string> cruft4;
	left=cruft3.begin();
	while (left != cruft3.end()) {
		right=explain.begin();
		bool match=false;
		while (right != explain.end()) {
			match=(*left==*right);
			if (match) break;
			right++;
		}
		if (!match) cruft4.push_back(*left);
		left++;
	}

	//cruft3.clear();
	if (debug) cout << cruft4.size() << " files in cruft4 database" << endl << flush;

	cout << "---- missing: dpkg ----" << endl;
	for (int i=0;i<missing.size();i++) {
		cout << "        " << missing[i] << endl;
	}

	//TODO: split by filesystem
	cout << "---- unexplained: / ----" << endl;
	for (int i=0;i<cruft4.size();i++) {
		cout << "        " << cruft4[i] << endl;
	}

	// NOT IMPLEMENTED
	cout << "---- broken symlinks: / ----" << endl;
	cout << endl << "end." << endl;
	return 0;
}
