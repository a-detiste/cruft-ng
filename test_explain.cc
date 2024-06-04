#include <iostream>
#include "dpkg.h"
#include "explain.h"
#include "owner.h"

int main()
{
	vector<string> packages;
	vector<owner> explain;
	dpkg_start("/");
	read_dpkg_header(packages);
	dpkg_end();
	read_explain("/etc/cruft/explain/", packages, explain);
	for (unsigned int i=0;i<explain.size();i++) {
		cout << explain[i].package << ' ' << explain[i].path << endl;
	}
}
