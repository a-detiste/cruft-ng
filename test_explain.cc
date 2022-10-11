#include <iostream>
#include "dpkg.h"
#include "explain.h"

int main(int argc, char *argv[])
{
	vector<string> packages,explain,useless;
	read_dpkg(packages,useless);
	read_explain(packages,explain);
	for (unsigned int i=0;i<explain.size();i++) {
		cout << explain[i] << endl;
	}
}
