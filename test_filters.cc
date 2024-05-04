#include <iostream>
#include "dpkg.h"
#include "filters.h"
#include "owner.h"

int main()
{
	vector<string> packages;
	vector<owner> globs;
	dpkg_start();
	read_dpkg_header(packages);
	dpkg_end();
	read_filters("/etc/cruft/filters/", "/usr/share/cruft/ruleset", packages, globs);
	for (unsigned int i=0;i<globs.size();i++) {
		cout << globs[i].package << ' ' << globs[i].path << endl;
	}
}
