#include <iostream>
#include "plocate.h"

int main()
{
	vector<string> fs;
	read_plocate(fs, "/etc/cruft/ignore");

	cout << '/' << endl;
	for (unsigned int i=0;i<fs.size();i++) {
		cout << fs[i] << endl;
	}
}
