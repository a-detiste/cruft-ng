#define LIBDPKG_VOLATILE_API
#include <iostream>
#include <algorithm>

#include <sys/stat.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-fsys.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-show.h>

#include "dpkg.h"
#include "usr_merge.h"

/*
https://www.dpkg.org/doc/libdpkg/structpkginfo.html
*/

int read_dpkg(vector<string>& packages, vector<string>& output)
{
	int i;
	struct pkg_array array;
	struct pkginfo *pkg;
	struct fsys_namenode_list *file;
	struct fsys_namenode *namenode;
	bool debug = getenv("DEBUG") != NULL;
	struct stat buffer;

	dpkg_program_init("cruft");
	modstatdb_open(msdbrw_readonly);
	//modstatdb_open(msdbrw_available_readonly);
	pkg_array_init_from_hash(&array);
	pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);

	for (i = 0; i < array.n_pkgs; i++) {
		pkg = array.pkgs[i];
		if (pkg->status == PKG_STAT_INSTALLED) {
			packages.push_back(pkg->set->name);
			if (debug) cout << "PACKAGE:" << pkg->set->name << endl;
			ensure_packagefiles_available(pkg);
			ensure_diversions();
			file = pkg->files;
			while (file) {
				namenode = file->namenode;
				if (debug) cout << namenode->name << endl;
				if (namenode->divert) {
					// We trust DPKG state for now
					if (stat(namenode->name, &buffer) == 0) {
						output.push_back(usr_merge(namenode->name));
					}
					if (stat(namenode->divert->useinstead->name, &buffer) == 0) {
						output.push_back(usr_merge(namenode->divert->useinstead->name));
					}
				} else {
					output.push_back(usr_merge(namenode->name));
				}
				file = file->next;
		        }
		}
	}
	modstatdb_shutdown();
	dpkg_program_done();
	sort(output.begin(), output.end());
	output.erase( unique( output.begin(), output.end() ), output.end() );
	return 0;
}
