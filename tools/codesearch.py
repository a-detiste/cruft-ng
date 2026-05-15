#!/usr/bin/python3

import os
import subprocess
import sys

try:
   PATH = sys.argv[1]
   assert PATH in ('cache', 'games', 'lib', 'log', 'spool')
except:
   exit('tools/codesearch.py cache|games|lib|log|spool')

import apt
CACHE = apt.Cache()

def bin2src(binaries: list[str]) -> list[str]:
    '''compare source packages with source packages'''
    sources = []
    for binary in binaries:
        try:
            package = CACHE[binary]
            source = package.candidate.source_name
        except KeyError:
            #print('!!! %s' % binary) # does not exists in Debian
            source = binary
        sources.append(source)
    return sources

done = subprocess.check_output(['apt-file', 'search', '--package-only', '/usr/share/cruft/rules/'], text=True).splitlines()
pending = os.listdir('rules/') + os.listdir('explain/')
pending = [p for p in sorted(pending) if p.lower() == p]  # ignore DH_CRUFT & DEBOOTSTRAP
todo = subprocess.check_output(['codesearch', 'path:postrm /var/%s' % PATH, '--only', '--max-result', '500'], text=True).splitlines()

done = bin2src(done)
pending = bin2src(pending)
to_check = set(todo) - set(done) - set(pending)
print('\n'.join(sorted(to_check)))
