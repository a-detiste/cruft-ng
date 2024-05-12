#!/usr/bin/python3

import glob
import re
import sys

import debianbts as bts

# Shell: /bin/sh linked to /bin/dash
FALSE_POSITIVES = set(['/bin/sh', '/bin/dash',
                   '/usr/bin/sh', '/usr/bin/dash',
                   '/usr/sbin/piuparts',
                   '/usr/share/fonts',
                  ])

for dpkg_list in glob.glob('/var/lib/dpkg/info/*.list') + glob.glob('/var/lib/dpkg/info/*.conffiles'):
    with open(dpkg_list, 'r') as files:
        for file in files:
            FALSE_POSITIVES.add(file.rstrip('\n'))

# exclude "'" because it's added by dpkg in error messages
re_cruft = re.compile(r'/(?:bin|usr|etc|var|\.cache)/[A-Za-z0-9_\/\.\-]*')

# https://udd.debian.org/cgi-bin/bts-usertags.cgi?user=cruft-ng@packages.debian.org&tag=cruft
if len(sys.argv) > 1:
    bugs = [int(bug) for bug in sys.argv[1:]]
else:
    bugs = bts.get_usertag('cruft-ng@packages.debian.org', tags=['cruft'])['cruft']

cruft = dict()

todo: set[int] = set(bugs)
done: set[int] = set()

while todo:
    todo -= done
    for bug in bts.get_status(list(todo)):
        todo.update(set(bug.mergedwith))
        done.add(bug.bug_num)
        for match in re.findall(re_cruft, bug.subject):
            match = match.rstrip('./')
            if match in cruft:
                continue
            cruft[match] = (bug.bug_num, bug.source, bug.subject)
        for mail in bts.get_bug_log(bug.bug_num):
            for line in mail['body'].splitlines():
                if '---- missing: dpkg ----' in line:
                    # bug report contains cruft report, ignore from here
                    break
                for match in re.findall(re_cruft, line):
                    if match in FALSE_POSITIVES:
                        continue
                    if match.startswith('/var/lib/dpkg/'):
                        continue
                    if '  interest-noawait ' in line:
                        continue
                    match = match.rstrip('./')
                    if match in cruft:
                        # keep original bug report
                        continue
                    cruft[match] = (bug.bug_num, bug.source, line)

for path,data in sorted(cruft.items()):
    bug_nr, package, raw = data
    #print(raw)
    print('%s %s %s' % (path, bug_nr, package))
