#!/usr/bin/python3

import os
import subprocess

STABLE = os.path.basename(os.readlink('archive/stable'))

for package in subprocess.check_output(['apt-file', 'search', '--package-only', '/usr/share/cruft/rules/'],
                                       text=True).splitlines():
    rule = os.path.join('rules', package)
    archive = 'archive/%s/%s' % (STABLE, package)
    if os.path.isfile(rule) and not os.path.isfile(archive):
        print('%s has been dh-cruft enabled' % package)
        subprocess.call(['git', 'mv', rule, archive])
