#include <vector>
#include <string>
using namespace std;

void dpkg_start( const string& root_dir);
void dpkg_end();
int query(const char *path);

int read_dpkg_header(vector<string>& packages);
int read_dpkg(vector<string>& packages, vector<string>& db, bool print_csv, const string& root_dir);

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
