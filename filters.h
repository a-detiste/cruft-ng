#include <vector>
#include <string>

#include "owner.h"

int read_filters(const std::string& dir, const std::string& ruleset_file, const std::vector<std::string>& packages, std::vector<owner>& globs);
