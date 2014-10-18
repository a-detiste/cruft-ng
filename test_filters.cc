#include <iostream>
#include "dpkg.h"
#include "filters.h"

int main(int argc, char *argv[])
{
	vector<string> packages,globs;
	read_dpkg_header(packages);
	read_filters(packages,globs);
	for (int i=0;i<globs.size();i++) {
		cout << globs[i] << endl;
	}
}
