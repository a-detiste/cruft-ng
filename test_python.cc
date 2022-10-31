#include <iostream>
#include "python.h"

void test(string path) {
	cout << "checking: " << path << endl;
	if (pyc_has_py(path, true)) {
		cout << "OK" << endl;
	} else {
		cout << "ERROR" << endl;
	};
}

int main()
{
	test("/usr/share/games/game-data-packager/game_data_packager/__pycache__");
	test("/home/tchet/git/pyposdisplay/pyposdisplay/pyposdisplay.pyc");
	test("/usr/share/python3/debpython/__pycache__/option.cpython-310.pyc");

	if (!pyc_has_py("/home/tchet/git/pyposdisplay/pyposdisplay/pyposdisplay.py", true)) {
		cout << "OK" << endl;
	} else {
		cout << "ERROR" << endl;
	};
}
