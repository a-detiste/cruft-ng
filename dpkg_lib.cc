#define LIBDPKG_VOLATILE_API
#include <iostream>

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-array.h>
#include "dpkg.h"

int read_dpkg_header(vector<string>& packages)
{
	struct pkg_array array;
	dpkg_program_init("cruft");
	modstatdb_open(msdbrw_available_readonly);
	pkg_array_init_from_hash(&array);
	cout << "packages: " << array.n_pkgs << endl;
	dpkg_program_done();
	return -1;
}
int read_dpkg_items(vector<string>& dpkg)
{
	return -1;
}
