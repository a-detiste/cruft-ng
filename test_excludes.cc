#include <iostream>
#include "dpkg_exclude.h"

int main(int argc, char *argv[])
{
	vector<string> packages,globs;
	read_dpkg_excludes(globs);
	for (unsigned int i=0;i<globs.size();i++) {
		cout << globs[i] << endl;
	}
}
