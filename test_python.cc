#include <iostream>
#include "python.h"

void test(string path, bool expected) {
	cout << "checking: " << path << endl;
	if (pyc_has_py(path, true) == expected) {
		cout << "\033[1;32mOK\033[0m" << endl << endl;
	} else {
		cout << "\033[1;31mERROR\033[0m" << endl << endl;
	};
}

int main()
{
	test("/usr/share/games/game-data-packager/game_data_packager/__pycache__", true);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-310.pyc", false);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-311.pyc", true);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-312.pyc", true);
	test("/usr/share/python3/debpython/__pycache__/option.cpython-313.pyc", false);
}
