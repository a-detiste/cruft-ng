#include <iostream>
#include "dpkg.h"
#include "explain.h"

int main(int argc, char *argv[])
{
	vector<string> packages,explain;
	read_dpkg_header(packages);
	read_explain(packages,explain);
	for (unsigned int i=0;i<explain.size();i++) {
		cout << explain[i] << endl;
	}
}
