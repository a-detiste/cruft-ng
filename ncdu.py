#!/usr/bin/python3

# cpigs -C | grep ^/ | sort -k1,1 -t ';' -u > cpigs.csv

# https://github.com/wodny/ncdu-export
# (GPL-3: but this file is not meant for distriubtion)

import os
import sys
import time
from itertools import takewhile
from operator import eq

print('[1,0,{"progname": "cpigs", "progver": "0.9", "timestamp": %s },' % int(time.time()))
print('[{"name":"/"}', end='') # not the ','

prev_dirs = []

# carefuly selected number to have a few files from /etc too
HEAD = 300
count = 0

HEAD = 2000
#HEAD = 99999999999

bug = """
{"name": "xml-core.xml", "dsize": 840},
{"name": "xml-core.xml.old", "dsize": 673}]],
[{"name": ""},
{"name": "initrd.img", "dsize": 1024},
{"name": "initrd.img.old", "dsize": 1024}],
[{"name": "lib64"},
{"name": "ld-linux-x86-64.so.2", "dsize": 223152}],
[{"name": ""},
{"name": "libx32", "dsize": 1024},
[{"name": "lost+found"}],
[{"name": "media"}]],
"""

def compare_dirs(dirs, prev_dirs):
    common_len = len(list(takewhile(lambda x: eq(*x), zip(dirs, prev_dirs))))
    closed = len(prev_dirs) - common_len
    opened = len(dirs) - common_len
    return closed, opened

def adjust_depth(dirs, prev_dirs):
    closed, opened = compare_dirs(dirs, prev_dirs)
    if closed:
        print("]"*closed, end="")
    if opened:
        for opened_dir in dirs[-opened:]:
            print(',\n[{"name": "%s"}' % opened_dir, end="")

with open('cpigs.csv', 'r') as dump:
    for line in dump:
        # "package" and "cruft" metadata are not supported in NCDU format
        path, _, type_, _, size = line.rstrip('\n').split(';')
        path = path.replace('"','_')

        basename = os.path.basename(path)
        if type == 'd':
            dirname = path
        else:
            dirname = os.path.dirname(path)

        dirs = dirname.lstrip("/").split("/")
        adjust_depth(dirs, prev_dirs)
        if type_ == 'd':
            print(',\n[{"name": "%s"}' % basename , end="")
            dirs.append(basename)
        else:
            print(',\n{"name": "%s", "dsize": %s}' % (basename, size), end="")
        prev_dirs = dirs

        # debug code
        count += 1
        if count > HEAD:
            break


dirs = []
adjust_depth(dirs, prev_dirs)

print(']]')

# ./ncdu.py 2>/dev/null | ncdu -f - --color dark
