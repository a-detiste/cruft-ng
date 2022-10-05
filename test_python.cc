#include <iostream>
#include "python.h"

int main(int argc, char *argv[])
{
	if (pyc_has_py("/usr/share/games/game-data-packager/game_data_packager/__pycache__")) {
		cout << "OK" << endl;
        } else {
		cout << "ERROR" << endl;
        };
}
