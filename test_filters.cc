#include <iostream>
#include "dpkg.h"
#include "filters.h"

int main(int argc, char *argv[])
{
	vector<string> packages,useless;
	vector<owner> globs;
	read_dpkg(packages,useless);
	read_filters(packages,globs);
	for (unsigned int i=0;i<globs.size();i++) {
		//cout << globs[i].glob << endl;
		cout << globs[i].package << " " << globs[i].glob  << endl;
	}
}
