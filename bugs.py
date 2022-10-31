#!/usr/bin/python3

import re
import sys

import debianbts as bts

# Shell: /bin/sh linked to /bin/dash
FALSE_POSITIVES = ['/bin/sh', '/bin/dash',
                   '/usr/bin/sh', '/usr/bin/dash',
                  ]

# exclude "'" because it's added by dpkg in error messages
re_cruft = re.compile(r'/(?:bin|usr|etc|var)/[A-Za-z0-9_\/\.\-]*')

# https://udd.debian.org/cgi-bin/bts-usertags.cgi?user=cruft-ng@packages.debian.org&tag=cruft
if len(sys.argv) > 1:
    bugs = [int(bug) for bug in sys.argv[1:]]
else:
    bugs = bts.get_usertag('cruft-ng@packages.debian.org', tags=['cruft'])['cruft']

cruft = dict()

for bug in bts.get_status(bugs):
    #print(bug)
    for mail in bts.get_bug_log(bug.bug_num):
        for line in mail['body'].splitlines():
            for match in re.findall(re_cruft, line):
                if match in FALSE_POSITIVES:
                    continue
                if match.startswith('/var/lib/dpkg/'):
                    continue
                if '  interest-noawait ' in line:
                    continue
                match = match.rstrip('.')
                cruft[match] = (bug.bug_num, bug.source, line)

for path,data in sorted(cruft.items()):
    bug, package, raw = data
    #print(raw)
    print('%s %s %s' % (path, bug, package))
