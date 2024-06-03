// Copyright © 2024 Jochen Sprickerhof <jspricke@debian.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <vector>
#include <string>

#ifndef NOLOCATE_H
#define NOLOCATE_H
using namespace std;

int read_nolocate(vector<string>& fs, const string& ignore_path);
#endif
