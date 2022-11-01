#!/usr/bin/python3

# cpigs -C | grep ^/ | sort -k1,1 -t ';' -u > cpigs.csv

import os
import sys
import time

print('[1,0,{"progname": "cpigs", "progver": "0.9", "timestamp": %s },' % int(time.time()))
print('[{"name":"/"}', end='') # not the ','

level = 0
location = ''

# carefuly selected number to have a few files from /etc too
HEAD = 300
count = 0

with open('cpigs.csv', 'r') as dump:
    for line in dump:
        # "package" and "cruft" metadata are not supported in NCDU format
        path, _, type_, _, size = line.rstrip('\n').split(';')
        path = path.replace('"','_')

        dirname = os.path.dirname(path)
        basename = os.path.basename(path)

        w = ' ' * (level+1)
        if type_ != 'd':
            if dirname == location:
                # we stay in the same dir, we just add more files
                print(',\n%s' % w, end='')
                print('{"name": "%s", "dsize": %s}' % (basename, size), end='')
            elif os.path.dirname(dirname) == location:
                # we go down one dir
                level +=1
                w = ' ' * (level+1)
                print(',\n%s' % w, end='')
                print('[{"name": "%s"},' % dirname)
                print(w, end='')
                print('{"name": "%s", "dsize": %s}' % (basename, size), end='')
                location = dirname
            elif dirname.startswith(location + '/'):
                # we need to go down further
                # and also generate intermediary dirs
                steps = dirname[len(location)+1:]
                print('steps', steps, file=sys.stderr)
                for step in steps.split('/'):
                    w = ' ' * (level+1)
                    print(',\n%s' % w, end='')
                    print('[{"name": "%s"}' % step, end='')
                    level += 1
                print(',\n%s' % w, end='')
                print('{"name": "%s", "dsize": %s}' % (basename, size), end='')
                location = dirname
            else:
                # we go up
                while not (dirname+'/').startswith(location + '/'):
                    location = os.path.dirname(location)
                    level -= 1
                    print(']', end='')
                    if level < 0:
                        location = '/'
                        break

                steps = dirname[len(location):]
                for step in steps[:len(steps)-1].split('/'):
                    if not step:
                        break
                    w = ' ' * (level+1)
                    print(',\n%s' % w, end='')
                    print('[{"name": "%s"}' % step, end='')
                    level +=1
                location = dirname

                print(',\n%s' % w, end='')
                print('{"name": "%s", "dsize": %s}' % (basename, size), end='')


        count += 1
        if count > HEAD:
            break

print(']' * level)

print(']]')

# ./ncdu.py 2>/dev/null | ncdu -f -
