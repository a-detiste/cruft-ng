#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include "usr_merge.h"

static bool check_link(const string& path)
{
	struct stat file_info;
	if (lstat(path.c_str(), &file_info) < 0) {
		cerr << "Failed to stat '" << path << "': " << strerror(errno) << '\n';
		return false;
	}
	return S_ISLNK(file_info.st_mode);
}

string usr_merge(const string& path)
{
	static bool setup = false;
	static bool MERGED_BIN, MERGED_LIB, MERGED_LIB32, MERGED_SBIN;
	if (!setup)
	{
		MERGED_BIN=check_link("/bin");
		MERGED_LIB=check_link("/lib");
		MERGED_LIB32=check_link("/lib32");
		MERGED_SBIN=check_link("/sbin");
		setup=true;
	}

	if ((MERGED_BIN and path.rfind("/bin/", 0) == 0)
	 or (MERGED_LIB and path.rfind("/lib/", 0) == 0)
	 or (MERGED_LIB32 and path.rfind("/lib32/", 0) == 0)
	 or (MERGED_SBIN and path.rfind("/sbin/", 0) == 0))
		return "/usr" + path;
	else
		return path;
}
