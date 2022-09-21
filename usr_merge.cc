#include <string>
#include <sys/stat.h>
#include "usr_merge.h"

bool check_link(string path)
{
	struct stat file_info;
	lstat(path.c_str(), &file_info);
	return S_ISLNK(file_info.st_mode);
}

string usr_merge(string path)
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

	if ((MERGED_BIN and path.substr(0,5)=="/bin/")
	 or (MERGED_LIB and path.substr(0,5)=="/lib/")
	 or (MERGED_LIB32 and path.substr(0,5)=="/lib32/")
	 or (MERGED_SBIN and path.substr(0,6)=="/sbin/"))
		return "/usr" + path;
	else
		return path;
}
