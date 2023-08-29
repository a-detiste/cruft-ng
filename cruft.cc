// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <thread>

#include <sys/stat.h>
#include <getopt.h>
#include <cstring>
#include <unistd.h>
#include <dirent.h>

#include "explain.h"
#include "filters.h"
#include "locate.h"
#include "dpkg.h"
#include "dpkg_exclude.h"
#include "shellexp.h"
#include "bugs.h"

using namespace std;

#ifdef BUSTER
#define LOCATE_DB "/var/lib/mlocate/mlocate.db"
#define UPDATEDB "updatedb.mlocate"
#else
#define LOCATE_DB "/var/lib/plocate/plocate.db"
#define UPDATEDB "updatedb.plocate"
#endif

/*
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
*/

static bool updatedb()
{
	/* return value is meant as an are_we_up_to_date flag
	   when running as non-root */
	const string db = LOCATE_DB;

	int rc_locate, rc_dpkg;
	struct stat stat_locate, stat_dpkg;
	rc_locate = stat(db.c_str(), &stat_locate);
	rc_dpkg = stat("/var/lib/dpkg/status", &stat_dpkg);

	if (rc_dpkg) {
		cerr << "can't read /var/lib/dpkg/status timestamp !!!\n";
		exit(1);
	}

	if (!rc_locate && stat_locate.st_mtim.tv_sec > stat_dpkg.st_mtim.tv_sec)
		return true;

	if (getuid()) return false;

	if (system(UPDATEDB)) {
		cerr << "updatedb failed\n";
		exit(1);
	}
	return true;
}

static void one_file(const string& path)
{
	string infile = path;
	if ((path.rfind("/bin/", 0) == 0)
         or (path.rfind("/lib/", 0) == 0)
         or (path.rfind("/sbin/", 0) == 0)) {
		infile = "/usr" + infile;
	}

	char* file = realpath(infile.c_str(), nullptr);

	vector<string> packages;
	vector<string> dpkg;
	read_dpkg(packages, dpkg, false);
	// is it a static file ?
	// TODO

	// is it a dynamic file ?
	vector<owner> globs;
	read_filters("/etc/cruft/filters/", "/usr/share/cruft/ruleset", packages, globs);
	for (const auto& gl: globs) {
		if (myglob(file, gl.path)) {
			cout << gl.package << '\n';
			return;
		};
	}

	// match the dynamic "explain" filters
	vector<owner> explain;
	read_explain("/etc/cruft/explain/", packages, explain);
	for (const auto& ex: explain) {
		if (infile == ex.path) {
			cout << ex.package << '\n';
			return;
                }
        }

	cerr << "no matching package found\n";
}

static int one_package(const string& package)
{
	vector<string> packages;
	packages.push_back(package);
	vector<owner> cruft;
	read_filters("/etc/cruft/filters/", "/usr/share/cruft/ruleset", packages, cruft);
	packages.push_back("binfmt-support");
	packages.push_back("ucf");
	read_explain("/etc/cruft/explain/", packages, cruft);
	for (const auto& cr: cruft) {
		if (cr.package == package) cout << cr.path << '\n';
	}
	return 0;
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

static const char* const default_explain_dir = "/etc/cruft/explain/";
static const char* const default_filter_dir = "/etc/cruft/filters/";
static const char* const default_ignore_file = "/etc/cruft/ignore";
static const char* const default_ruleset_file = "/usr/share/cruft/ruleset";
static const char* const default_bugs_file = "/usr/share/cruft/bugs";

static void print_help_message()
{
	cout << "cruft-ng [OPTIONS] [file]\n\n";
	cout << "if <file> is specified, this file is analysed\n";
	cout << "if not, the whole system is analysed\n\n";

	cout << "OPTIONS\n";
	cout << "    -p --package     list volatile files only for this one package\n";
	cout << "    -E --explain     directory for explain scripts (default: " << default_explain_dir << ")\n";
	cout << "    -F --filter      directory for filters (default: " << default_filter_dir << ")\n";
	cout << "    -I --ignore      path for ignore file (default: " << default_ignore_file << ")\n";
	cout << "    -R --ruleset     path for ruleset file (default: " << default_ruleset_file << ")\n";
	cout << "    -B --bugs        path for known bugs file (default: " << default_bugs_file << ")\n";

	cout << '\n';

	cout << "    -h --help        this help message\n";
}

static void cruft(const string& ignore_file,
                  const string& filter_dir,
                  const string& ruleset_file,
                  const string& explain_dir)
{
	bool debug = getenv("DEBUG") != nullptr;

	const int SIZEBUF = 200;
	char buf[SIZEBUF];
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo=localtime(&rawtime);
	setlocale(LC_TIME, "");
	strftime(buf, sizeof(buf), "%c", timeinfo);
	cout << "cruft report: " << buf << '\n';

	bool updated = updatedb();
	if (!updated) {
		cerr << "warning: plocate database is outdated" << endl << flush;
	}
	elapsed("updatedb");

	vector<string> fs;
	thread thr_plocate(read_locate, ref(fs), ignore_file);

	vector<string> packages;
	vector<string> dpkg;
	thread thr_dpkg(read_dpkg, ref(packages), ref(dpkg), false);
	thr_plocate.join();
	thr_dpkg.join();
	elapsed("plocate + dpkg");

	map<string, bug> bugs;
	read_bugs(bugs, "bugs");

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
	read_filters(filter_dir, ruleset_file, packages, globs);
	elapsed("read filters");
	vector<string> cruft3;
	for (const auto& cr: cruft) {
		bool match=false;
		for (const auto& gl: globs) {
			match=myglob(cr, gl.path);
			if (match) break;
		}
		if (!match) cruft3.push_back(cr);
	}
	elapsed("extra vs globs");
	if (debug) cerr << cruft3.size() << " files in cruft3 database\n\n";

	// match the dynamic "explain" filters
	vector<owner> explain;
	read_explain(explain_dir, packages, explain);
	elapsed("read explain");
	vector<string> cruft4;
	for (const auto& cr: cruft3) {
		bool match=false;
		for (const auto& ex: explain) {
			match=(cr==ex.path);
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
		cout << "        " << cr;
		auto bug = bugs.find(cr);
		if (bug != bugs.end()) {
			cout << "       (Bug: #" << bug->second.bugno << ")";
		}
		cout << '\n';
	}

	cout << "\nend.\n";
	exit(0);
}

int main(int argc, char *argv[])
{
	bool do_one_package = false;
	string package = "";
	string explain_dir = default_explain_dir;
	string filter_dir = default_filter_dir;
	string ignore_file = default_ignore_file;
	string ruleset_file = default_ruleset_file;
	string bugs_file = default_bugs_file;

	const struct option long_options[] =
	{
		{"help", no_argument, nullptr, 'h'},
		{"package", required_argument, nullptr, 'p'},
		{"explain", required_argument, nullptr, 'E'},
		{"filter", required_argument, nullptr, 'F'},
		{"ignore", required_argument, nullptr, 'I'},
		{"ruleset", required_argument, nullptr, 'R'},
		{"bugs", required_argument, nullptr, 'B'},
		{0, 0, 0, 0}
	};

	int opt, opti = 0;
	while ((opt = getopt_long(argc, argv, "p:E:F:hI:R:B:", long_options, &opti)) != 0) {
		if (opt == EOF)
			break;

		switch (opt) {
		case 'p':
			do_one_package = true;
			package = optarg;
			break;
		case 'E':
			explain_dir = optarg;
			if (!explain_dir.empty() && explain_dir.back() != '/')
				explain_dir += '/';
			break;

		case 'F':
			filter_dir = optarg;
			if (!filter_dir.empty() && filter_dir.back() != '/')
				filter_dir += '/';
			break;

		case 'h':
			print_help_message();
			exit(0);

		case 'I':
			ignore_file = optarg;
			break;

		case 'R':
			ruleset_file = optarg;
			break;

		case 'B':
			bugs_file = optarg;
			break;

		case '?':
			print_help_message();
			exit(1);

	        default:
	            cerr << "Invalid getopt return value: " << opt << "\n";
				break;
		}
	}


	if (do_one_package) exit(one_package(package));

	if (optind < argc) {
		if (optind + 1 == argc) {
			struct stat buffer;
			if (stat(argv[1], &buffer) == 0) {
				one_file(argv[1]);
				exit(0);
			} else {
				cerr << "file not found\n";
				exit(1);
			}
		}

		cerr << "Invalid non-option arguments:";
		while (optind < argc)
			cerr << " " << argv[optind++];
		cerr << '\n';
		print_help_message();
		exit(1);
	}

	// else: standard cruft report
	cruft(ignore_file, filter_dir, ruleset_file, explain_dir);
}
