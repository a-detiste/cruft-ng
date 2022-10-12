#!/usr/bin/perl
use warnings;
use strict;

use Debian::Debhelper::Dh_Lib;

insert_after('dh_lintian', 'dh_cruft');

1;
