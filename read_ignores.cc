// Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <fstream>

#include "read_ignores.h"

void read_ignores(vector<string>& ignores, const string& ignore_path)
{
	ifstream ignore_file(ignore_path);
	if (!ignore_file.is_open())
		ignore_file.open("/usr/share/cruft/ignore");

	for (string ignore_line; getline(ignore_file,ignore_line);)
	{
		if (ignore_line.empty()) continue;
		if (ignore_line.front() == '/') {
			if (ignore_line.back() != '/')
				ignore_line += "/";
			ignores.emplace_back(std::move(ignore_line));
		}
	}
}
