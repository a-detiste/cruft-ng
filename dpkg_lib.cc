// Copyright © 2022 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#define LIBDPKG_VOLATILE_API
#include <array>
#include <iostream>
#include <algorithm>

extern "C" {
#include <sys/stat.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-fsys.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-list.h>
#include <dpkg/pkg-show.h>
}

#include "dpkg.h"
#include "usr_merge.h"

/*
https://www.dpkg.org/doc/libdpkg/structpkginfo.html
*/

void dpkg_start() {
	dpkg_program_init("cruft");
	modstatdb_open(msdbrw_readonly);
}

void dpkg_end() {
	modstatdb_shutdown();
	dpkg_program_done();
}

int query(const char *path)
{
	ensure_allinstfiles_available_quiet();
	ensure_diversions();

	struct fsys_namenode *namenode;
	namenode = fsys_hash_find_node(path, FHFF_NOCOPY);

	if (namenode->divert) {
	        cout << namenode->divert->pkgset->name << '\n';
		return 1;
	} else if(namenode->packages) {
		cout << namenode->packages->pkg->set->name << '\n';
		return 1;
	}
	return 0;
}


int read_dpkg_header(vector<string>& packages)
{
	struct pkg_array array;
	pkg_array_init_from_hash(&array);
	pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);

	int i;
	for (i = 0; i < array.n_pkgs; i++) {
		struct pkginfo *pkg;

		pkg = array.pkgs[i];
		if (pkg->status == PKG_STAT_INSTALLED || pkg->status == PKG_STAT_CONFIGFILES)
			packages.emplace_back(pkg->set->name);
	}

	pkg_array_destroy(&array);
	return 0;
}


static void csv(const char *dpkg_name, string realname, const char *package)
{
	struct stat st;
	if (stat(dpkg_name, &st) != 0)
		return;
	if (S_ISDIR(st.st_mode))
		return;

	char type;
	type = S_ISLNK(st.st_mode) ? 'l' : 'f';

	replace(realname.begin(), realname.end(), ';', '_');

	cout    << realname << ';'
		<< package << ';'
		<< type << ';'
		<< '0' << ';'
		<< st.st_size << '\n';
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
	struct pkg_array array;
	struct pkginfo *pkg;
	bool debug = getenv("DEBUG_DPKG") != nullptr;
	struct stat buffer;

	admindir = dpkg_db_set_dir(admindir);

#ifdef DB_API
	var_lib_dpkg_info = &output;
#else
	const std::array<string, 16> suffixes {
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
#endif

	pkg_array_init_from_hash(&array);
	pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);
	ensure_diversions();
	ensure_allinstfiles_available_quiet();

	for (int i = 0; i < array.n_pkgs; i++) {
		pkg = array.pkgs[i];
		if (pkg->status == PKG_STAT_INSTALLED || pkg->status == PKG_STAT_CONFIGFILES) {
			if(pkg->status == PKG_STAT_INSTALLED) {
				packages.emplace_back(pkg->set->name);
			}
			if (debug) cout << "PACKAGE:" << pkg->set->name << '\n';

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

			for (const auto& suffix: suffixes) {
				string control = control_ + suffix;
				if (stat(control.c_str(), &buffer) == 0) {
					output.emplace_back(control);
				}
			}
#endif

			for (const struct fsys_namenode_list *file = pkg->files; file != nullptr; file = file->next) {
				const struct fsys_namenode *namenode = file->namenode;
				if (debug) cout << namenode->name << '\n';
				if (namenode->divert) {
					// We trust DPKG state for now
					if (stat(namenode->name, &buffer) == 0) {
						string realname = usr_merge(namenode->name);
						if (print_csv) csv(namenode->name, realname, pkg->set->name);
						output.emplace_back(std::move(realname));
					}
					if (stat(namenode->divert->useinstead->name, &buffer) == 0) {
						string realname = usr_merge(namenode->divert->useinstead->name);
						if (print_csv) csv(namenode->name, realname, pkg->set->name);
						output.emplace_back(std::move(realname));
					}
				} else {
					string realname = usr_merge(namenode->name);
					if (print_csv) csv(namenode->name, realname, pkg->set->name);
					output.emplace_back(std::move(realname));
				}
			}
		}
	}
	pkg_array_destroy(&array);
	sort(output.begin(), output.end());
	output.erase( unique( output.begin(), output.end() ), output.end() );
	return 0;
}
