#include <iostream>
#include <map>
#include <algorithm>
#include <ctime>

#include <jsoncpp/json/json.h>
#include <fnmatch.h>
#include <string.h>
#include <time.h>

#include "explain.h"
#include "filters.h"
#include "plocate.h"
#include "dpkg.h"

extern "C" int shellexp(char* filename, char* pattern );

using namespace std;

#ifndef BUSTER
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
using namespace std::experimental;
namespace fs = std::experimental::filesystem;
#endif

bool myglob(string file, string glob )
{
	//stack<string> st;

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
	cerr << "usage: " << endl;
	cerr << "  cpigs [-n] [NUMBER]  : default format" << endl;
	cerr << "  cpigs -e             : export in ncdu format" << endl;
	cerr << "  cpigs -c             : export in .csv format" << endl;
	//cerr << "  cpigs -C             : export in .csv format, also static files" << endl;
	return 1;
}

void output_pigs(long unsigned int limit, map<string, int>& usage)
{
	vector<pair<string,int>> pigs;
	copy(usage.begin(), usage.end(), back_inserter<vector<pair<string,int>>> (pigs));
	sort(pigs.rbegin(), pigs.rend(), [](const std::pair<string,int> &left,
					    const std::pair<string,int> &right) {
		return left.second < right.second;
	});
	for (size_t i = 0; i < pigs.size() && i < limit; ++i) {
		if (pigs[i].second > 0) cout << pigs[i].second << " " << pigs[i].first << endl;
	}
}

void output_csv(vector<string>& cruft_db)
{
}

void output_ncdu(vector<string>& cruft_db)
{
	Json::Value json(Json::arrayValue);
	json.append(1);
	json.append(2);
	Json::Value signature(Json::objectValue);
	signature["progname"] = "cpigs";
	signature["progver"] = "0";
	signature["timestamp"] = int(time(nullptr));
	json.append(signature);
	Json::Value root(Json::arrayValue);

	/*
	Json::Value dir(Json::objectValue);
	dir["name"] = "/";
	dir["asize"] = 4096;
	dir["dsize"] = 4096;
	root.append(dir);

	Json::Value file(Json::objectValue);
	file["name"] = "HELLO";
	file["asize"] = 13;
	file["dsize"] = 4096;
	root.append(file);

	Json::Value file2(Json::objectValue);
	file2["name"] = "WORLD";
	file2["asize"] = 42;
	file2["dsize"] = 4096;
	root.append(file2);


	Json::Value newroot(Json::arrayValue);
	Json::Value subdir(Json::objectValue);
	subdir["name"] = "etc";
	subdir["asize"] = 4096;
	subdir["dsize"] = 4096;
	newroot.append(subdir);
	root.append(newroot);
	https://en.wikibooks.org/wiki/JsonCpp#Example
	*/

	string dir;

	vector<string>::iterator cruft;
	for (cruft=cruft_db.begin();cruft !=cruft_db.end();cruft++)
	{
		string dest = *cruft;
		if (fs::is_directory(*cruft)) {
			cerr << dest << endl;
			if (dest.size() > dir.size() && !dest.compare(0, dir.size(), dir)) {
				// we go down
				size_t pos = string::npos;
				size_t prev_pos = dir.size()+1;
				string step;
				while ((pos = dest.find('/', prev_pos)) != string::npos) {
					string step = dest.substr(prev_pos, pos - prev_pos);
					cerr << "+ " <<step << endl;
					prev_pos = pos + 1;
				}
				step = dest.substr(prev_pos);
				cerr << "+ " << step << endl;
				dir = dest;
			} else {
				// we go up
				size_t pos = dir.size();
				size_t prev_pos = string::npos;
				string step;
				while ((pos = dir.rfind('/', prev_pos))) {
					if (!dir.compare(0, pos - 2, dest)) break;
					string step = dir.substr(pos + 1, prev_pos - pos);
					cerr << "- " << step << endl;
					prev_pos = pos - 1;
				}

				// we go back up again
				prev_pos += 2;
				while ((pos = dest.find('/', prev_pos)) != string::npos) {
                                        string step = dest.substr(prev_pos, pos - prev_pos);
                                        cerr << "+ " <<step << endl;
                                        prev_pos = pos + 1;
                                }
				step = dest.substr(prev_pos);
				cerr << "+ " << step << endl;

				dir = dest;
			}
		} else if (fs::is_symlink(*cruft)) {
			//
		} else {
			//
		}
	}

	json.append(root);
	cout << json.toStyledString() << endl;
}

int main(int argc, char *argv[])
{
	long unsigned int limit = 10;

	bool ncdu = false, csv = false /*, static_ = false*/;
	if (argc == 2 && !strcmp(argv[1], "-e")) {
		ncdu = true;
	} else if (argc == 2 && !strcmp(argv[1], "-c")) {
		csv = true;
	//} else if (argc == 2 && !strcmp(argv[1], "-C")) {
	//	csv = true;
	//	static_ = true;
	} else if (argc == 3) {
		try {
			limit = stoi(argv[2]);
		} catch(...) { return usage(); }
	} else if (argc == 2) {
		try {
			limit = stoi(argv[1]);
		} catch(...) { return usage(); }
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

	if (ncdu) {
		output_ncdu(cruft_db);
		return 0;
	};

	if (csv) cout << "path;package;type;cruft;size" << endl;

	vector<owner> globs;
	read_filters(packages,globs);
	read_explain(packages,globs);
	elapsed("read filters");

        std::map<std::string, int> usage{{"UNKNOWN", 0}};

	vector<string>::iterator cruft=cruft_db.begin();
	vector<owner>::iterator owners;
	while (cruft != cruft_db.end()) {
		owners = globs.begin();
		string package = "UNKNOWN";
		while (owners != globs.end()) {
			bool match;
			match = myglob(*cruft,(*owners).glob);
			if (match) {
				package = (*owners).package;
                        	break;
                	}
			owners++;
		}

		char type;
		size_t fsize;
		try
		{
			if (fs::is_symlink(*cruft)) {
				type = 'l';
				fsize = 1024;
			} else if (fs::is_directory(*cruft)) {
				type = 'd';
				fsize = 1024;
			} else {
				type = 'f';
				fsize = fs::file_size(*cruft);
			}
		}
		catch (...) {
			type = '?';
			fsize = 1024;
		}

		if (csv) {
			cout << *cruft << ";" << package << ";" << type << ";1;" << fsize << endl;
		} else {
			if (usage.count(package) == 0) usage[package] = 0;
			usage[package] += fsize;
		}

		cruft++;
	}
	elapsed("extra vs globs");

	output_pigs(limit, usage);

	return 0;
}
