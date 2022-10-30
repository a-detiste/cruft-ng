#include <vector>
#include <string>
using namespace std;

int read_dpkg(vector<string>& packages, vector<string>& db, bool print_csv);

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
