#!/usr/bin/python3

import glob
import os

for list in glob.glob('/var/lib/dpkg/info/*.list') + glob.glob('/var/lib/dpkg/info/*.conffiles'):
    package = os.path.basename(list)
    package = package.split(':')[0]
    package = package.split('.')[0]
    with open(list, 'r') as files:
        for file in files:
            file = file.rstrip('\n')
            if os.path.isdir(file):
                continue
            elif os.path.islink(file):
                type_ = 'l'
                size = 1024
            else:
                type_ = 'f'
                size = os.stat(file).st_size
            print("%s;%s;%s;0;%s" % (file, package, type_, size))
