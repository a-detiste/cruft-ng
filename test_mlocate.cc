#include "mlocate.h"

int main(int argc, char *argv[])
{
	std::vector<string> fs;
	read_mlocate(fs);
	for (int i=0;i<fs.size();i++) {
		cout << fs[i] << endl;
	}
}

/*
 * sudo locate / | grep -v ^/root/ | grep -v ^/usr/local/ | grep -v ^/home/ | sort -u > /tmp/db1
 * sudo /home/tchet/cruft-ng/test_mlocate | sort -u > /tmp/db2
 * colordiff -u db1 db2  | less -R
 * 
 */