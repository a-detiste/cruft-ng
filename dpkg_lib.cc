#define LIBDPKG_VOLATILE_API
#include <iostream>
#include <algorithm>

#include <sys/stat.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-show.h>

#include "dpkg.h"
#include "usr_merge.h"

/*
https://www.dpkg.org/doc/libdpkg/structpkginfo.html
*/

static const char *admindir;

#ifdef DB_API
vector<string>* var_lib_dpkg_info = NULL;

static void callback(const char *filename, const char *filetype)
{
	var_lib_dpkg_info->push_back(filename);
}
#endif

int read_dpkg(vector<string>& packages, vector<string>& output)
{
	int i;
	struct pkg_array array;
	struct pkginfo *pkg;
	struct fsys_namenode_list *file;
	struct fsys_namenode *namenode;
	bool debug = getenv("DEBUG") != NULL;
	struct stat buffer;

	admindir = dpkg_db_set_dir(admindir);

#ifdef DB_API
	var_lib_dpkg_info = &output;
#else
	vector<string> suffixes {
		".list",
		".md5sums",
		".postinst",
		".postrm",
		".preinst",
		".prerm",
		".shlibs",
		".symbols",
		".triggers",
	};
	vector<string>::iterator suffix;
#endif

	dpkg_program_init("cruft");
	modstatdb_open(msdbrw_readonly);
	pkg_array_init_from_hash(&array);
	pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);
	ensure_diversions();
	ensure_allinstfiles_available_quiet();

	for (i = 0; i < array.n_pkgs; i++) {
		pkg = array.pkgs[i];
		if (pkg->status == PKG_STAT_INSTALLED) {
			packages.push_back(pkg->set->name);
			if (debug) cout << "PACKAGE:" << pkg->set->name << endl;

#ifdef DB_API
			// this is just too slow, from 2 seconds to 40
			// I hope this will go away in the big DPKG rewrite
			pkg_infodb_foreach(pkg, &pkg->installed, callback);
#else
			suffix = suffixes.begin();
			while(suffix != suffixes.end() ) {
				string control = admindir;
				control += "/info/";
				control += pkg_name(pkg, pnaw_nonambig);
				control += *suffix;
				if (stat(control.c_str(), &buffer) == 0) {
					output.push_back(control);
				}
				suffix++;
			}
#endif

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
