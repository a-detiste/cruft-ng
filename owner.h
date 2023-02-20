#include <string>

#ifndef OWNER_H
#define OWNER_H
struct owner
{
	std::string package;
	std::string path;
	owner(std::string, std::string);
};

bool operator==(owner const&, owner const&);
bool operator<(owner const&, owner const&);
#endif
