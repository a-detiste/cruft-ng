#!/usr/bin/python3

BASE = '/usr/share/cruft/rules'

import glob
import os
import subprocess

STABLE = os.path.basename(os.readlink('archive/stable'))

for rule in glob.glob('rules/*'):
    package = os.path.basename(rule)
    dest = os.path.join(BASE, package)
    archive = 'archive/%s/%s' % (STABLE, package)
    if os.path.isfile(dest) and not os.path.exists(archive):
        print('%s has been dh-cruft enabled' % package)
        subprocess.call(['diff', '-u', rule, dest])
        subprocess.call(['git', 'mv', rule, archive])
