#include <string>

#ifndef OWNER_H
#define OWNER_H
struct owner
{
	std::string package;
	std::string glob;
	owner(std::string, std::string);
	bool operator==(owner const&);
};
#endif
