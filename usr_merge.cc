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
	static bool MERGED_BIN, MERGED_LIB, MERGED_LIB32, MERGED_LIB64, MERGED_LIBO32, MERGED_LIBX32, MERGED_SBIN;
	if (!setup)
	{
		MERGED_BIN=check_link("/bin", true);
		MERGED_LIB=check_link("/lib", true);
		MERGED_LIB32=check_link("/lib32", false);
		MERGED_LIB64=check_link("/lib64", false);
		MERGED_LIBO32=check_link("/libo32", false);
		MERGED_LIBX32=check_link("/libx32", false);
		MERGED_SBIN=check_link("/sbin", true);
		setup=true;
	}

	if ((MERGED_BIN and path.rfind("/bin/", 0) == 0)
	 or (MERGED_LIB and path.rfind("/lib/", 0) == 0)
	 or (MERGED_LIB32 and path.rfind("/lib32/", 0) == 0)
	 or (MERGED_LIB64 and path.rfind("/lib64/", 0) == 0)
	 or (MERGED_LIBO32 and path.rfind("/libo32/", 0) == 0)
	 or (MERGED_LIBX32 and path.rfind("/libx32/", 0) == 0)
	 or (MERGED_SBIN and path.rfind("/sbin/", 0) == 0))
		return "/usr" + path;
	else
		return path;
}
