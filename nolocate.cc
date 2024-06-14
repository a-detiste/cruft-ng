// Copyright © 2024 Jochen Sprickerhof <jspricke@debian.org>
// SPDX-License-Identifier: GPL-2.0-or-later

// new code: do not attempt to support Buster or Hurd

#include <iostream>
#include <algorithm>
#include <filesystem>

#include <sys/vfs.h>
#include <linux/magic.h>

#include "nolocate.h"
#include "python.h"
#include "read_ignores.h"

using namespace std;

int read_nolocate(vector<string>& fs, const string& ignore_path, const string& root_dir)
{
	bool debug=getenv("DEBUG") != nullptr;

	if (debug) cerr << "FILESYSTEM DATA\n";

	init_python();

	vector<string> ignores;
	read_ignores(ignores, ignore_path);

	struct statfs buf;
	auto root_dir_length = root_dir.length()-1;

        for (auto entry =
                 filesystem::recursive_directory_iterator{
                     root_dir,
                     filesystem::directory_options::skip_permission_denied};
             entry != filesystem::recursive_directory_iterator(); entry++)
	{
		std::string filename{entry->path(), root_dir_length};

		statfs(filename.c_str(), &buf);

		if (buf.f_type == SYSFS_MAGIC
		    or buf.f_type == PROC_SUPER_MAGIC
		    or filename == "/dev"
		    or (filename == "/home" /* and dirname != "/home" */)
		    or filename == "/media"
		    or filename == "/mnt"
		    or filename == "/root"
		    or filename == "/tmp")
			entry.disable_recursion_pending();

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
			&& filesystem::is_directory(filename, ec)) {
				ignored = true;
				break;
			}
		}
		if (ignored) continue;

		if (!pyc_has_py(string{entry->path()}, debug))
			fs.emplace_back(filename);
	}

	sort(fs.begin(), fs.end());
	fs.erase( unique( fs.begin(), fs.end() ), fs.end() );
	if (debug) cerr << fs.size() << " relevant files in filesystem"  << endl << endl;
	return 0;
}
