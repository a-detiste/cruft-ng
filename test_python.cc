#include <iostream>
#include "python.h"

int main(int argc, char *argv[])
{
	if (pyc_has_py("/usr/share/games/game-data-packager/game_data_packager/__pycache__")) {
		cout << "OK" << endl;
	} else {
		cout << "ERROR" << endl;
	};

	// my last Python2 file
	if (pyc_has_py("/home/tchet/git/pyposdisplay/pyposdisplay/pyposdisplay.pyc")) {
		cout << "OK" << endl;
	} else {
		cout << "ERROR" << endl;
	};

	if (pyc_has_py("/usr/share/python3/debpython/__pycache__/option.cpython-310.pyc")) {
		cout << "OK" << endl;
	} else {
		cout << "ERROR" << endl;
	};

	if (!pyc_has_py("/home/tchet/git/pyposdisplay/pyposdisplay/pyposdisplay.py")) {
		cout << "OK" << endl;
	} else {
		cout << "ERROR" << endl;
	};
}
