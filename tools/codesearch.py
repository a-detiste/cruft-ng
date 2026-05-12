#!/usr/bin/python3

import os
import subprocess

PATH = '/var/cache'

done = subprocess.check_output(['apt-file', 'search', '--package-only', '/usr/share/cruft/rules/'], text=True).splitlines()
pending = os.listdir('rules/')
todo = subprocess.check_output(['codesearch', 'path:postrm %s' % PATH, '--only'], text=True).splitlines()

# BUG: mix binary vs source package
to_check = set(todo) - set(done) - set(pending)
print('\n'.join(sorted(to_check)))
