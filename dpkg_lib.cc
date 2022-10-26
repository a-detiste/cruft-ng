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

void csv(const char *dpkg_name,
         string realname,
         const char *package) {

	struct stat st;
	if (!stat(dpkg_name, &st) == 0)
		return;
	if (S_ISDIR(st.st_mode))
		return;

	char type;
	if (S_ISLNK(st.st_mode)) type = 'l';
	else type = 'f';

	replace(realname.begin(), realname.end(), ';', '_');

	cout    << realname << ';'
		<< package << ';'
		<< type << ';'
		<< '0' << ';'
		<< st.st_size << endl;
}

static const char *admindir;

#ifdef DB_API
#include <dpkg/db-ctrl.h>

vector<string>* var_lib_dpkg_info = NULL;

static void callback(const char *filename, const char *filetype)
{
	var_lib_dpkg_info->push_back(filename);
}
#endif

int read_dpkg(vector<string>& packages, vector<string>& output, bool print_csv)
{
	int i;
	struct pkg_array array;
	struct pkginfo *pkg;
	struct fsys_namenode_list *file;
	struct fsys_namenode *namenode;
	bool debug = getenv("DEBUG_DPKG") != NULL;
	struct stat buffer;

	admindir = dpkg_db_set_dir(admindir);

#ifdef DB_API
	var_lib_dpkg_info = &output;
#else
	vector<string> suffixes {
		".clilibs",
		".conffiles",
		".config",
		".fortran_mod",
		".list",
		".md5sums",
		".postinst",
		".postrm",
		".preinst",
		".prerm",
		".runit",
		".shlibs",
		".starlibs",
		".symbols",
		".templates",
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
		if (pkg->status == PKG_STAT_INSTALLED || pkg->status == PKG_STAT_CONFIGFILES) {
			if(pkg->status == PKG_STAT_INSTALLED) {
				packages.push_back(pkg->set->name);
			}
			if (debug) cout << "PACKAGE:" << pkg->set->name << endl;

#ifdef DB_API
			// this is just too slow, from 2 seconds to 40
			// I hope this will go away in the big DPKG rewrite
			pkg_infodb_foreach(pkg, &pkg->installed, callback);
#else
			string control_ = admindir;
			control_ += "/info/";
			// not ok for i386 only packages:
			//  steam, steamcmd, zsnes ...
			//control += pkg_name(pkg, pnaw_nonambig);
			control_ += pkg->set->name;
			if (pkg->installed.multiarch == PKG_MULTIARCH_SAME) {
				control_ += ":";
				control_ += pkg->installed.arch->name;
			}

			suffix = suffixes.begin();
			while(suffix != suffixes.end() ) {
				string control;
				control = control_;
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
				string realname;
				if (namenode->divert) {
					// We trust DPKG state for now
					if (stat(namenode->name, &buffer) == 0) {
						realname = usr_merge(namenode->name);
						output.push_back(realname);
					}
					if (stat(namenode->divert->useinstead->name, &buffer) == 0) {
						realname = usr_merge(namenode->divert->useinstead->name);
						output.push_back(realname);
					}
				} else {
					realname = usr_merge(namenode->name);
					output.push_back(realname);
				}

				if (print_csv) csv(namenode->name, realname, pkg->set->name);

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
