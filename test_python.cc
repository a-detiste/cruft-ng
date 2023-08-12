#include <iostream>
#include "python.h"

#define GREEN "\033[1;32m"
#define RED "\033[1;31m"
#define BLACK "\033[0m"

void test(string path, bool expected) {
	cout << "checking: " << path << endl;
	if (pyc_has_py(path, true) == expected) {
		cout << GREEN << "OK" << BLACK << endl << endl;
	} else {
		cout << RED << "ERROR" << BLACK << endl << endl;
	};
}

int main()
{
	init_python();
	cout << "installed versions:" << endl;
	for (unsigned int i=0;i<versions.size();i++) {
		cout << GREEN << versions[i] << BLACK << endl;
	}
	cout << endl;

	test("/usr/share/games/game-data-packager/game_data_packager/__pycache__", true);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-310.pyc", false);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-311.pyc", true);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-312.pyc", true);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-313.pyc", false);
}
