// Copyright © 2022 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include <map>
#include <algorithm>
#include <ctime>

#include <string.h>
#include <time.h>

#include "explain.h"
#include "filters.h"
#include "plocate.h"
#include "dpkg.h"
#include "shellexp.h"

using namespace std;

#ifndef BUSTER
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
using namespace std::experimental;
namespace fs = std::experimental::filesystem;
#endif

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
	//cerr << "  cpigs -e             : export in ncdu format" << endl;
	cerr << "  cpigs -c             : export in .csv format" << endl;
	cerr << "  cpigs -C             : export in .csv format, also static files" << endl;
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

void output_ncdu(vector<string>& cruft_db)
{
	// https://dev.yorhel.nl/ncdu/jsonfmt
	// https://github.com/rofl0r/ncdu/blob/master/src/dir_export.c

	cout << "[1,0,{\"progname\": \"cpigs\", \"progver\": \"0.9\",";
	cout << "\"timestamp\": " <<  int(time(nullptr)) << "},\n";

	cout << "[{\"name\":\"/\"}"; // not the ','

	string dir;

	/*
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
	*/

	cout << "]]" << endl;
}

int main(int argc, char *argv[])
{
	long unsigned int limit = 10;

	bool ncdu = false, csv = false, static_ = false;
	if (argc == 2 && !strcmp(argv[1], "-e")) {
		ncdu = true;
	} else if (argc == 2 && !strcmp(argv[1], "-c")) {
		csv = true;
	} else if (argc == 2 && !strcmp(argv[1], "-C")) {
		csv = true;
		static_ = true;
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
	read_plocate(fs,prunefs, "/etc/cruft/ignore");
	elapsed("plocate");

	if (csv) cout << "path;package;type;cruft;size" << endl;

	vector<string> packages;
	vector<string> dpkg;
	read_dpkg(packages, dpkg, static_);
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

	vector<owner> globs;
	read_filters("/etc/cruft/filters/", "/usr/share/cruft/ruleset", packages,globs);
	read_explain("/etc/cruft/explain/", packages,globs);
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
			cout << *cruft << ';' << package << ';' << type << ";1;" << fsize << endl;
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
