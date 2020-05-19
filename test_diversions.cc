#include <iostream>
#include <fstream>

#include "dpkg.h"

int read_diversions_old(vector<Diversion>& diversions)
{
        ifstream txt("/var/lib/dpkg/diversions");
        while(!txt.eof())
        {
                string oldfile,newfile,package;
                getline(txt,oldfile);
                getline(txt,newfile);
                getline(txt,package);
                diversions.push_back(Diversion(oldfile,newfile,package));
        }
        txt.close();
        return 0;
}


int main(int argc, char *argv[])
{
	vector<Diversion> diversions1;
	read_diversions_old(diversions1);

	for (unsigned int i=0;i<diversions1.size();i++) {
		cout << diversions1[i].oldfile << endl;
		cout << diversions1[i].newfile << endl;
		cout << diversions1[i].package << endl;
	}

	cout << "----------------------" << endl;

	vector<Diversion> diversions2;
	read_diversions(diversions2);
	for (unsigned int i=0;i<diversions2.size();i++) {
		cout << diversions2[i].oldfile << endl;
		cout << diversions2[i].newfile << endl;
		cout << diversions2[i].package << endl;
	}

}
