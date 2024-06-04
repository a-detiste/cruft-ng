#include <iostream>
#include "dpkg.h"

int main()
{
	vector<string> packages;
	vector<string> db;
	dpkg_start("/");
	read_dpkg(packages, db, false, "/");
	dpkg_end();

	for (unsigned int i=0;i<packages.size();i++) {
		cout << packages[i] << endl;
	}
	cout << "######" << endl;

	cout << "/" << endl;
	for (unsigned int i=0;i<db.size();i++) {
		cout << db[i] << endl;
	}
}
