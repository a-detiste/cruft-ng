#include <iostream>
#include "dpkg.h"

int main(int argc, char *argv[])
{
	vector<string> packages;
	vector<string> db;
	read_dpkg(packages, db);

	for (unsigned int i=0;i<packages.size();i++) {
		cout << packages[i] << endl;
	}
	cout << endl;

	cout << '/' << endl;
	for (unsigned int i=0;i<db.size();i++) {
		cout << db[i] << endl;
	}
}
