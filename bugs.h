#pragma once

#include <map>
#include <string>

struct bug
{
	std::string package;
	std::string bugno;
	bug(std::string, std::string);
};

bool operator<(bug const&, bug const&);

void read_bugs(std::map<std::string, bug>& bugs, const std::string& bugs_path);
