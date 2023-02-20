// Copyright © 2022 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "owner.h"

using namespace std;

owner::owner( std::string package_, std::string path_ )
{
        package = std::move(package_);
        path = std::move(path_);
}

bool operator==(owner const& l, owner const &r)
{
    return l.path == r.path;
}

bool operator<(owner const& l, owner const &r)
{
    return l.path < r.path;
}
