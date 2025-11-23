#!/usr/bin/python3
# encoding=utf-8
#
# Copyright © 2015 Alexandre Detiste <alexandre@detiste.be>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# https://launchpad.net/~alexandre-detiste/+archive/ubuntu/ppa

import os
import subprocess
import sys
import time

import distro_info

CRUFT = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
BASE = os.path.dirname(CRUFT)

info = distro_info.UbuntuDistroInfo()
stable = str(info.stable())
lts = str(info.lts())
old_lts = str(info.supported()[-3])

releases = sorted(set([old_lts, lts, stable]))
if len(sys.argv) > 1:
    release = sys.argv[1]
    assert release in releases
    releases = [release]

print('RELEASES:', releases)

subprocess.check_call(['git', 'checkout', 'debian/changelog'],
                       cwd = CRUFT)
subprocess.check_call(['git', 'checkout', 'debian/control'],
                       cwd = CRUFT)

from debian.changelog import Changelog
with open('debian/changelog', encoding='utf-8') as log:
    cl = Changelog(log, strict=False)

assert cl.distributions in ('unstable','UNRELEASED'), cl.distributions

if cl.distributions == 'unstable':
   build_type = 'Backport to PPA'
   build_number = 'release+'
else:
   build_type = 'Git snapshot'
   build_number = time.strftime('git%Y%m%d+')

with open('debian/control', 'r') as compat:
    for line in compat:
        if 'debhelper-compat' in line:
            current_debhelper =  int(line.split('(')[1].strip(' =),\n'))
            break

for release in sorted(releases):

    supported_debhelper = {
            'focal': 12,
            }.get(release, current_debhelper)
    BACKPORT = supported_debhelper < current_debhelper

    if BACKPORT:
        build_dep = 'debhelper-compat ( = %d)' %  supported_debhelper
        subprocess.check_call(['sed', '-i',
                               r's/\ *debhelper-compat.*/ ' + build_dep + ',/',
                               'debian/control'],
                              cwd = CRUFT)

    snapshot = str(cl.version).split('~')[0] + '~' + build_number + release
    subprocess.check_call(['dch', '-b',
                           '-D', release,
                           '-v', snapshot,
                           build_type],
                          cwd = CRUFT)

    subprocess.check_call(['debuild', '--no-lintian', '-S', '-i'], cwd = CRUFT)
    subprocess.check_call(['dput', 'my-ppa',
                           'cruft-ng_%s_source.changes' % snapshot],
                           cwd = BASE)

    subprocess.check_call(['git', 'checkout', 'debian/changelog'],
                          cwd = CRUFT)
    if BACKPORT:
        subprocess.check_call(['git', 'checkout', 'debian/control'],
                               cwd = CRUFT)
        subprocess.check_call(['git', 'checkout', 'debian/dh-cruft.manpages'],
                               cwd = CRUFT)

    for file in ('.tar.xz',
                 '.dsc',
                 '_source.build',
                 '_source.changes',
                 '_source.my-ppa.upload'):
        subprocess.check_call(['rm', '-v',
                               'cruft-ng_%s%s' % (snapshot, file)],
                              cwd = BASE)
