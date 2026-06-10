#!/usr/bin/python3

# the idea: if the package maintainer uses tmpfiles,
#           they are likely to accept a /purge too

import os
import subprocess

todo = set(os.listdir('/home/tchet/deb/cruft-ng/rules/'))
tmpfiles = set(subprocess.check_output(['apt-file', 'search', '--package-only', '/usr/lib/tmpfiles.d/'], text=True).splitlines())

# descope "systemd"
done = set(subprocess.check_output(['apt-file', 'search', '--package-only', '/usr/share/cruft/rules/'], text=True).splitlines())

to_check = (todo & tmpfiles) - done
print('\n'.join(sorted(to_check)))
