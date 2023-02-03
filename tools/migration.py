#!/usr/bin/python3

BASE = '/usr/share/cruft/rules'

import glob
import os
import subprocess

for rule in glob.glob('rules/*'):
    package = os.path.basename(rule)
    dest = os.path.join(BASE, package)
    if os.path.isfile(dest):
        print('%s has been dh-cruft enabled' % package)
        subprocess.call(['diff', '-u', rule, dest])
        subprocess.call(['git', 'mv', rule, 'archive/stable'])
