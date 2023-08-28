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
#include "locate.h"
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
	cerr << "  cpigs -e             : export in ncdu format" << endl;
	cerr << "  cpigs -c             : export in .csv format" << endl;
	cerr << "  cpigs -C             : export in .csv format, also static files" << endl;
	return 1;
}

static void output_pigs(long unsigned int limit, const map<string, size_t>& usage)
{
	vector<pair<string,size_t>> pigs;
	copy(usage.begin(), usage.end(), back_inserter(pigs));
	sort(pigs.rbegin(), pigs.rend(), [](const auto &left,
					    const auto &right) {
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

	fs::path last_dir = "/";

	vector<string>::iterator it;
	for (it=cruft_db.begin(); it != cruft_db.end(); it++)
	{
		fs::path cruft = *it;
		fs::path dirname;
		error_code ec;

		if (fs::is_directory(cruft, ec)) {
			dirname = cruft;
		} else {
			dirname = cruft.parent_path();
		}

		if (last_dir != dirname)
		{
			auto l = last_dir.begin();
			auto d = dirname.begin();
			int common_len = 0;
			while(l != last_dir.end() && d != dirname.end() && *l == *d)
			{
				common_len++;
				l++;
				d++;
			}

			int len_last_dir = 0;
			for(l = last_dir.begin(); l != last_dir.end() ;l++) len_last_dir++;

			int closed = len_last_dir - common_len;
			for(int c = closed; c; c--) cout << ']';

			int skipped = 0;
			for(auto& part : dirname)
			{
				if (skipped >= common_len) {
				        cout << ",\n[{\"name\":" << part << "}";
				}
				skipped++;
			}
			last_dir = dirname;
		}

		if (!fs::is_directory(cruft, ec)) {
			fs::path basename = cruft.filename();
			cout << ",\n{\"name\":" << basename;
			try {
				if(fs::is_symlink(cruft)) {
					// some arbitrary value
					// still better than  /var/cache/pbuilder/base.cow/dev/core -> /proc/kcore
					// showing up as 128 TiB and dwarfing everything else
					cout << ",\"dsize\":1024";
				} else {
					auto fsize = fs::file_size(cruft);
					cout << ",\"dsize\":" << fsize;
				}
			}
			catch (...) {}
			cout << "}";
		}
	}

	for(auto& part : last_dir) { cout << ']'; }
	cout << "]" << endl;
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

	vector<string> fs;
	read_locate(fs, "/usr/share/cruft/ignore");
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
	read_filters("/etc/cruft/filters/", "/usr/share/cruft/ruleset", packages, globs);
	read_explain("/etc/cruft/explain/", packages, globs);
	elapsed("read filters");

	std::map<std::string, size_t> usage{{"UNKNOWN", 0}};

	vector<string>::iterator cruft=cruft_db.begin();
	vector<owner>::iterator owners;
	while (cruft != cruft_db.end()) {
		owners = globs.begin();
		string package = "UNKNOWN";
		while (owners != globs.end()) {
			bool match;
			match = myglob(*cruft,(*owners).path);
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
