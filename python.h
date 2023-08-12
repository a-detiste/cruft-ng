#include <string>
#include <vector>

using namespace std;

extern vector<string> versions;

void init_python();
bool pyc_has_py(string pyc, bool debug);
