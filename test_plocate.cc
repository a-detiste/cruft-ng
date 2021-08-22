#include <iostream>
#include "plocate.h"

int main(int argc, char *argv[])
{
	vector<string> fs,prunefs;
	read_plocate(fs,prunefs);

	cout << '/' << endl;
	for (unsigned int i=0;i<fs.size();i++) {
		cout << fs[i] << endl;
	}
}
