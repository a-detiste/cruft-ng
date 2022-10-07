#include <iostream>
#include "dpkg.h"

int main(int argc, char *argv[])
{
	vector<string> packages;
	read_dpkg_header(packages);

	vector<string> db;
	read_dpkg_items(db);

	cout << '/' << endl;
	for (unsigned int i=0;i<db.size();i++) {
		cout << db[i] << endl;
	}
}
