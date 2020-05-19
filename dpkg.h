#include <vector>
#include <string>
using namespace std;

int read_dpkg_header(vector<string>& packages);
int read_dpkg_items(vector<string>& dpkg);

struct Diversion{
        string oldfile;
        string newfile;
        string package;
        Diversion(string oldfile,string newfile,string package)
        {
                this->oldfile=oldfile;
                this->newfile=newfile;
                this->package=package;
        }
};

int read_diversions(vector<Diversion>& diversions);
