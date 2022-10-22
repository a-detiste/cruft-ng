#include <iostream>
#include <map>
#include <filesystem>
#include <algorithm>

#include <fnmatch.h>
#include <string.h>
#include <time.h>

#include "explain.h"
#include "filters.h"
#include "plocate.h"
#include "dpkg.h"

extern "C" int shellexp(char* filename, char* pattern );

using namespace std;
namespace fs = std::filesystem;

bool myglob(string file, string glob )
{
	bool debug = getenv("DEBUG_GLOB") != NULL;

	if (file==glob) return true;
	unsigned int filesize=file.size();
	unsigned int globsize=glob.size();
		if ( glob.find("**")==globsize-2
		  and filesize >= globsize-2
		  and file.substr(0,globsize-2)==glob.substr(0,globsize-2)) {
		if (debug) cerr << "match ** " << file << " # " << glob << endl;
		return true;
	}  else if ( glob.find("*")==globsize-1
		  and filesize >= globsize-1
		  and file.find("/",globsize-1)==string::npos
		  and file.substr(0,globsize-1)==glob.substr(0,globsize-1)) {
		if (debug) cerr << "match * " << file << " # " << glob << endl;
		return true;
	} else if ( fnmatch(glob.c_str(),file.c_str(),FNM_PATHNAME)==0 ) {
		if (debug) cerr << "fnmatch " << file << " # " << glob << endl;
		return true;
	} else {
		// fallback to shellexp.c
		char param1[256];
		strncpy(param1,file.c_str(),sizeof(param1));
		param1[sizeof(param1)-1] = '\0';
		char param2[256];
		strncpy(param2,glob.c_str(),sizeof(param2));
		param2[sizeof(param2)-1] = '\0';
		bool result=shellexp(param1,param2);
		if (result and debug) {
			cerr << "shellexp.c " << file << " # " << glob << endl;
		}
		return result;
	}
}

clock_t beg = clock();

void elapsed(string action)
{
	if (getenv("ELAPSED") == NULL) return;
	clock_t end = clock();
	double elapsed_seconds = (end - beg) * 1000 / CLOCKS_PER_SEC;
	cerr << "elapsed " << action << ": " << elapsed_seconds << endl;
	beg = end;
}

int usage()
{
	cerr << "usage: cpigs [-i] [NUMBER]" << endl;
	return 1;
}

int main(int argc, char *argv[])
{
	long unsigned int limit;
        if (argc == 3) {
		try {
			limit = stoi(argv[2]);
		} catch(...) { return usage(); }
        } else if (argc == 2) {
		try {
			limit = stoi(argv[1]);
		} catch(...) { return usage(); }
	} else {
		limit = 10;
	}

	vector<string> fs,prunefs;
	read_plocate(fs,prunefs);
	elapsed("plocate");

	vector<string> packages;
	vector<string> dpkg;
	read_dpkg(packages, dpkg);
	elapsed("dpkg");

	vector<string> cruft_db;
	vector<string>::iterator left=fs.begin();
	vector<string>::iterator right=dpkg.begin();
	while (left != fs.end() )
	{
		if (*left==*right) {
			left++;
			right++;
		} else if (*left < *right) {
			cruft_db.push_back(*left);
			left++;
		} else {
			right++;
		}
		if (right == dpkg.end())
                     while(left !=fs.end()) {cruft_db.push_back(*left); left++;};
	}
	elapsed("main set match");

	vector<owner> globs;
	read_filters(packages,globs);
	read_explain(packages,globs);
	elapsed("read filters");

        std::map<std::string, int> usage{{"UNKNOWN", 0}};

	vector<string>::iterator cruft=cruft_db.begin();
	vector<owner>::iterator owners;
	while (cruft != cruft_db.end()) {
		owners = globs.begin();
		bool match = false;
		while (owners != globs.end()) {
			match = myglob(*cruft,(*owners).glob);
			if (match) {
				if (usage.count((*owners).package) == 0) usage[(*owners).package] = 0;
				try
				{
					size_t fsize;
					if (fs::is_symlink(*cruft)) fsize=1024;
					else fsize = fs::file_size(*cruft);
					//cout << *cruft << "[" << fsize << "]" << endl;
					usage[(*owners).package] += fsize;
				}
				catch (...) { }
                        	break;
                	}
			owners++;
		}

		if (!match) {
			//cout << *cruft << endl;
			try
			{
				usage["UNKNOWN"] += fs::file_size(*cruft);
			}
			catch (...) { }
		}
		cruft++;
	}
	elapsed("extra vs globs");

	vector<pair<string,int>> pigs;
	copy(usage.begin(), usage.end(), back_inserter<vector<pair<string,int>>> (pigs));
	sort(pigs.rbegin(), pigs.rend(), [](const std::pair<string,int> &left,
					    const std::pair<string,int> &right) {
  		return left.second < right.second;
	});
	for (size_t i = 0; i < pigs.size() && i < limit; ++i) {
		if (pigs[i].second > 0) cout << pigs[i].first << " " << pigs[i].second << endl;
	}

	return 0;
}
