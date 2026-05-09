#!/usr/bin/python3

# rules that exists but do not match anything

import glob
import os

def is_installed(package: str) -> bool:
    for suffix in ('', ':amd64', ':i386'):
        if os.path.exists('/var/lib/dpkg/info/%s%s.list' % (package, suffix)):
            return True
    else:
        return False

for rule in sorted(glob.glob('rules/*')):
    package = os.path.basename(rule)
    if is_installed(package):
        with open(rule) as fd:
            for path in fd:
                path = path.strip('\n')
                if path.startswith('#'):
                    pass
                elif os.path.exists(path):
                    pass
                elif glob.glob(path):
                    pass
                else:
                    print(package, path)
