// Copyright © 2016 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include "usr_merge.h"

static bool check_link(const string& path, const bool mandatory)
{
	struct stat file_info;
	if (lstat(path.c_str(), &file_info) < 0) {
		if (mandatory) cerr << "Failed to stat '" << path << "': " << strerror(errno) << '\n';
		return false;
	}
	return S_ISLNK(file_info.st_mode);
}

string usr_merge(const string& path)
{
	static bool setup = false;
	static bool MERGED;
	if (!setup)
	{
		MERGED=check_link("/bin", true);
		setup=true;
	}

	if (MERGED and (path.rfind("/bin/", 0) == 0
			or path.rfind("/lib/", 0) == 0
			or path.rfind("/lib32/", 0) == 0
			or path.rfind("/lib62/", 0) == 0
			or path.rfind("/sbin/", 0) == 0))
		return "/usr" + path;
	else
		return path;
}
