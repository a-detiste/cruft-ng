#define LIBDPKG_VOLATILE_API

// gcc query.c $(pkgconf --static --libs libdpkg) -o query

#include <stdio.h>

#include <dpkg/dpkg.h>
#include <dpkg/db-fsys.h>
#include <dpkg/pkg-list.h>

int main(int argc, const char *const *argv)
{
    if(argc != 2) {
        fputs("usage: query path\n", stderr);
        return 1;
    }

    dpkg_program_init("query");
    modstatdb_open(msdbrw_readonly);
    ensure_allinstfiles_available_quiet();
    ensure_diversions();

    struct fsys_namenode *namenode;
    namenode = fsys_hash_find_node(argv[1], 0);

    if (namenode->divert) {
        fputs(namenode->divert->pkgset->name, stdout);
        fputs("\n", stdout);
    } else if(namenode->packages) {
        fputs(namenode->packages->pkg->set->name, stdout);
        fputs("\n", stdout);
    }

    modstatdb_shutdown();
    dpkg_program_done();
    return 0;
}
