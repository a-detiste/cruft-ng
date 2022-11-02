// Copyright © 2022 Alexandre Detiste <alexandre@detiste.be>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "owner.h"

using namespace std;

owner::owner( std::string package_, std::string glob_ )
{
        package = std::move(package_);
        glob = std::move(glob_);
}

bool operator==(owner const& l, owner const &r)
{
    return l.glob == r.glob;
}

bool operator<(owner const& l, owner const &r)
{
    return l.glob < r.glob;
}
