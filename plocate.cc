// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "locate.h"
#include "python.h"

#include <filesystem>
namespace fs = std::filesystem;

// build fail on hurd-i386
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static void read_ignores(vector<string>& ignores, const string& ignore_path)
{
	ifstream ignore_file(ignore_path);
	if (!ignore_file.is_open())
		ignore_file.open("/usr/share/cruft/ignore");

	for (string ignore_line; getline(ignore_file,ignore_line);)
	{
		if (ignore_line.empty()) continue;
		if (ignore_line.front() == '/') {
			if (ignore_line.back() != '/')
				ignore_line += "/";
			ignores.emplace_back(std::move(ignore_line));
		}
	}
}

int read_locate(vector<string>& fs, const string& ignore_path)
{
	bool debug=getenv("DEBUG") != nullptr;

	if (debug) cerr << "PLOCATE DATA\n";

	vector<string> ignores;
	read_ignores(ignores, ignore_path);

	fs.emplace_back("/.");
	fs.emplace_back("/dev");
	fs.emplace_back("/home");
	fs.emplace_back("/root");
	fs.emplace_back("/tmp");

	char buf[PATH_MAX];
	FILE* fp;
	if ((fp = popen("plocate /", "r")) == nullptr) return 1;
	while (fgets(buf, sizeof(buf),fp))
	{
#ifndef BUSTER
		auto len = strlen(buf);
		if (len == 0)
			continue;
		string_view filename { buf, len - 1 };  // trim trailing newline
#else
		buf[strlen(buf)-1] = '\0';
		string filename = buf;
#endif
		auto toplevel { filename.substr(0, filename.find('/', 1)) };
		if (   toplevel == "/dev"
		    or (toplevel == "/home" /* and dirname != "/home" */)
		    or toplevel == "/mnt"
		    or toplevel == "/root"
		    or toplevel == "/tmp")
			continue;

		bool ignored = false;
		for (const auto& it : ignores) {
			if (filename.size() > it.size() && filename.compare(0, it.size(), it) == 0) {
				ignored = true;
				break;
			}

			// ignore directory '/foo' for ignore entry '/foo/'
			error_code ec;
			if (filename.size() + 1 == it.size()
			&& it.compare(0, filename.size(), filename) == 0
			&& fs::is_directory(filename, ec)) {
				ignored = true;
				break;
			}
		}
		if (ignored) continue;

		if (!pyc_has_py(string{filename}, debug))
			fs.emplace_back(filename);
	}
	pclose(fp);

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
	if (debug) cerr << fs.size() << " relevant files in PLOCATE database"  << endl << endl;
	return 0;
}
