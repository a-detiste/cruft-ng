#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "mlocate_db.h"
//       original filename is 'db.h'
// TODO: should be packaged in /usr/include by 'mlocate' package
//       like 'make' provides '/usr/include/gnumake.h'
//       and  'sudo' provides '/usr/include/sudo_plugin.h'

using namespace std;

int read_mlocate(vector<string>& fs, vector<string>& prunefs);
