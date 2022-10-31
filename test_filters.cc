#include <iostream>
#include "dpkg.h"
#include "filters.h"
#include "owner.h"

int main()
{
	vector<string> packages,useless;
	vector<owner> globs;
	read_dpkg(packages, useless, false);
	read_filters("/etc/cruft/filters/", "/usr/share/cruft/ruleset", packages, globs);
	for (unsigned int i=0;i<globs.size();i++) {
		//cout << globs[i].glob << endl;
		cout << globs[i].package << ' ' << globs[i].glob  << endl;
	}
}
