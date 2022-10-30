#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

#include <sys/stat.h>
#include <fnmatch.h>
#include <cstring>
#include <unistd.h>
#include <dirent.h>

#include "explain.h"
#include "filters.h"
#include "plocate.h"
#include "dpkg.h"
#include "dpkg_exclude.h"

extern "C" int shellexp(char* filename, char* pattern );

using namespace std;

static int read_mounts(const vector<string>& prunefs, vector<string>& mounts)
{
	// this doesn't include "/", as it always exists
	ifstream mtab("/proc/mounts");
	string mount,type;
	while (mtab.good())
	{
		getline(mtab,mount,' '); // discard device
		getline(mtab,mount,' ');
		getline(mtab,type,' ');

		bool match=false;
		if (mount=="/") match=true;
		else if (mount.rfind("/sys/", 0) == 0) match=true;
		else if (mount.rfind("/dev/", 0) == 0) match=true;
		else {
			for (const auto& elem: prunefs) {
				string uppercase=type;
				transform(uppercase.begin(), uppercase.end(), uppercase.begin(), ::toupper);
				if (uppercase==elem) {
			  		match=true;
					break;
		      	}
	      	}
		}

		if (!match) {
			//cerr << mount << " => " << type << endl;
			mounts.emplace_back(mount);
		}
		getline(mtab,mount); // discard options
	}
	return 0;
}

static void updatedb(const string& db)
{
	if (getuid()) return;

	int rc_locate, rc_dpkg;
	struct stat stat_locate, stat_dpkg;
	rc_locate = stat(db.c_str(), &stat_locate);
	rc_dpkg = stat("/var/lib/dpkg/status", &stat_dpkg);

	if (rc_dpkg) {
		cerr << "can't read /var/lib/dpkg/status timestamp !!!\n";
		exit(1);
	}

	if (!rc_locate && stat_locate.st_mtim.tv_sec > stat_dpkg.st_mtim.tv_sec)
		return;

	if (system("updatedb")) {
		cerr << "updatedb failed\n";
		exit(1);
	}
}

static bool myglob(const string& file, const string& glob )
{
	bool debug = getenv("DEBUG_GLOB") != nullptr;

	if (file==glob) return true;
	auto filesize=file.size();
	auto globsize=glob.size();
		if ( glob.find("**")==globsize-2
		  and filesize >= globsize-2
		  and file.compare(0, globsize - 2, glob) == 0) {
		if (debug) cerr << "match ** " << file << " # " << glob << '\n';
		return true;
	}  else if ( glob.find('*')==globsize-1
		  and filesize >= globsize-1
		  and file.find('/',globsize-1)==string::npos
		  and file.compare(0, globsize - 1, glob) == 0) {
		if (debug) cerr << "match * " << file << " # " << glob << '\n';
		return true;
	} else if ( fnmatch(glob.c_str(),file.c_str(),FNM_PATHNAME)==0 ) {
		if (debug) cerr << "fnmatch " << file << " # " << glob << '\n';
		return true;
	} else {
		// fallback to shellexp.c
		bool result=shellexp(file.c_str(),glob.c_str());
		if (result and debug) {
			cerr << "shellexp.c " << file << " # " << glob << '\n';
		}
		return result;
	}
}

static void one_file(const string& infile)
{
	char* file=realpath(infile.c_str(), nullptr);
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir("/usr/lib/cruft/filters-unex/")) == nullptr) {
		cerr << "Error(" << errno << ") opening /usr/lib/cruft/filters-unex/\n";
		exit(1);
	}
	bool matched = false;
	while ((dirp = readdir(dp)) != nullptr) {
		string package=dirp->d_name;
		if (package == "." or package == "..") continue;
		ifstream glob_file("/usr/lib/cruft/filters-unex/" + package);
		for (string glob_line; getline(glob_file,glob_line);)
		{
			if (glob_line.empty()) continue;
			if (glob_line.front() == '/') {
				if (myglob(file,glob_line)) {
					cout << package << '\n';
					matched = true;
				}
			}
		}
	}
	closedir(dp);

	if (not matched) cerr << "no matching package found\n";
}

static clock_t beg = clock();

static void elapsed(const string& action)
{
	if (getenv("ELAPSED") == nullptr) return;
	clock_t end = clock();
	clock_t elapsed_mseconds = (end - beg) * 1000 / CLOCKS_PER_SEC;
	cerr << "elapsed " << action << ": " << elapsed_mseconds << '\n';
	beg = end;
}

int main(int argc, char *argv[])
{
	bool debug = getenv("DEBUG") != nullptr;

	if (argc == 2) {
		struct stat buffer;
		if (stat(argv[1], &buffer) == 0) {
			one_file(argv[1]);
			exit(0);
		} else {
			cerr << "file not found\n";
			exit(1);
		}
	}

	if (argc > 1) {
		cerr << "cruft-ng [file]\n\n";
		cerr << "if <file> is specified, this file is analysed\n";
		cerr << "if not, the whole system is analysed\n";
		exit(1);
	}

	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo=localtime(&rawtime);
	setlocale(LC_TIME, "");
	strftime(buf, sizeof(buf), "%c", timeinfo);
	cout << "cruft report: " << buf << '\n';

	updatedb("/var/lib/plocate/plocate.db");
	elapsed("updatedb");

	vector<string> fs,prunefs,mounts;
	read_plocate(fs,prunefs);
	read_mounts(prunefs,mounts);
	elapsed("plocate");

	vector<string> packages;
	vector<string> dpkg;
	read_dpkg(packages, dpkg, false);
	elapsed("dpkg");

	// match two main data sources
	vector<string> cruft;
	vector<string> missing;
	for (auto left=fs.begin(), right=dpkg.begin(); left != fs.end() && right != dpkg.end();)
	{
		//cerr << "[" << *left << "=" << *right << "]" << endl;
		if (*left==*right) {
			left++;
			right++;
		} else if (*left < *right) {
			cruft.push_back(*left);
			left++;
		} else {
			missing.push_back(*right);
			right++;
		}
		if (right == dpkg.end()) while(left  !=fs.end()  ) {cruft.push_back(*left);    left++; }
		if (left  == fs.end()  ) while(right !=dpkg.end()) {missing.push_back(*right); right++;}
	}
	elapsed("main set match");

	if (debug) cerr << missing.size() << " files in missing database\n";
	if (debug) cerr << cruft.size() << " files in cruft database\n\n";

	// https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=619086
	vector<string> excludes;
	read_dpkg_excludes(excludes);
	elapsed("read excludes");
	vector<string> missing2;
	unsigned long count_stat = 0;
	for (const auto& miss: missing) {
		bool match=false;
		for (const auto& ex: excludes) {
			match=myglob(miss,ex);
			if (match) break;
		}
		if (!match) {
			// file may exist on tmpfs
			// e.g.: /var/cache/apt/archives/partial
			struct stat stat_buffer;
	                if ( stat(miss.c_str(), &stat_buffer) == 0) {
				count_stat += 1;
				if (debug) cerr << miss << " was not in plocate database\n";
			} else {
				missing2.push_back(miss);
			}
		}
	}
	elapsed("missing2");
	if (debug) cerr << "count stat():" << count_stat << '\n';

	// match the globs against reduced database
	vector<owner> globs;
	read_filters(packages,globs);
	elapsed("read filters");
	vector<string> cruft3;
	for (const auto& cr: cruft) {
		bool match=false;
		for (const auto& gl: globs) {
			match=myglob(cr, gl.glob);
			if (match) break;
		}
		if (!match) cruft3.push_back(cr);
	}
	elapsed("extra vs globs");
	if (debug) cerr << cruft3.size() << " files in cruft3 database\n\n";

	// match the dynamic "explain" filters
	vector<owner> explain;
	read_explain(packages,explain);
	elapsed("read explain");
	vector<string> cruft4;
	for (const auto& cr: cruft3) {
		bool match=false;
		for (const auto& ex: explain) {
			match=(cr==ex.glob);
			if (match) break;
		}
		if (!match) cruft4.push_back(cr);
	}
	elapsed("extra vs explain");

	if (debug) cerr << cruft4.size() << " files in cruft4 database\n";

	//TODO: some smarter algo when run as non-root
        //      like checking the R/X bits of parent dir
	cout << "---- missing: dpkg ----\n";
	if (geteuid() == 0 ) for (const auto& miss: missing2) {
		cout << "        " << miss << '\n';
	}

	//TODO: split by filesystem
	cout << "---- unexplained: / ----\n";
	for (const auto& cr: cruft4) {
		cout << "        " << cr << '\n';
	}

	cout << "\nend.\n";
	return 0;
}
