#include <iostream>
#include "locate.h"

int main()
{
	vector<string> fs;
	read_locate(fs, "/etc/cruft/ignore");

	cout << '/' << endl;
	for (unsigned int i=0;i<fs.size();i++) {
		cout << fs[i] << endl;
	}
}
