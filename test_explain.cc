#include <iostream>
#include "dpkg.h"
#include "explain.h"
#include "owner.h"

int main()
{
	vector<string> packages,useless;
	vector<owner> explain;
	read_dpkg(packages, useless, false);
	read_explain("/etc/cruft/explain/", packages, explain);
	for (unsigned int i=0;i<explain.size();i++) {
		cout << explain[i].package << ' ' << explain[i].glob << endl;
	}
}
