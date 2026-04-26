#!/usr/bin/python3

from datetime import datetime
import os

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as mtick

from git import Repo

def graph(package: str, rules: str) -> tuple[list[datetime],list[int]]:
    repo = Repo.init('/home/tchet/deb/%s' % package)
    count = dict()

    for c in repo.iter_commits(all=True, paths=[rules]):
        blob = c.tree[rules]
        count[c.authored_date] = len(blob.list_traverse())

    stamps = []
    values = []
    for k, v in sorted(count.items()):
        stamps.append(datetime.fromtimestamp(k))
        values.append(v)
    return stamps, values

st_old, c_old = graph('cruft', 'filters-unex')
st_new, c_new = graph('cruft-ng', 'rules')

# add data from https://snapshot.debian.org/binary/cruft/
#
# for d in *.deb
# do
#   echo -n $(echo $d | cut -d_ -f 2) " "
#   dpkg --fsys-tarfile $d | tar -tl --wildcards '*usr/lib/cruft/filters*' | grep -v 'usr/lib/cruft/filters/$'| wc -l
# done
SEED: dict[datetime, int] = {
    datetime(1998, 4, 10): 81, # https://lists.debian.org/debian-policy/1998/04/msg00089.html
    datetime(1998, 11, 9): 112, # 0.9.5
    datetime(1999, 12, 19): 112, # 0.9.6-0.1
    datetime(2002, 11, 22): 112, # 0.9.6-0.4
    datetime(2005, 7, 16): 129, # 0.9.6-0.5
    datetime(2005, 7, 20): 129, # 0.9.6-0.6
    datetime(2005, 7, 25): 129, # 0.9.6-0.7
    datetime(2005, 8, 29): 172, # 0.9.6-0.8
    datetime(2005, 10, 25): 156, # 0.9.6-0.9
    datetime(2005, 10, 28): 175, # 0.9.6-0.10
    datetime(2005, 11, 2): 185, # 0.9.6-0.11
    datetime(2006, 2, 6): 208, # 0.9.6-0.13
    datetime(2006, 6, 8): 208, # 0.9.6-0.14
    datetime(2006, 6, 15): 208, # 0.9.6-0.15
    datetime(2007, 1, 18): 210, # 0.9.6-0.16
    datetime(2007, 3, 3): 210, # 0.9.6-0.17
}

# split cruft <-> cruft-common
CUTOFF = datetime(2015, 6, 29)
st_cruft = list()
c_cruft = list()
for k, v in sorted(SEED.items()):
    st_cruft.append(k)
    c_cruft.append(v)

st_common = list()
c_common = list()
for st, c in zip(st_old, c_old):
    if st < CUTOFF:
       st_cruft.append(st)
       c_cruft.append(c)
    else:
       st_common.append(st)
       c_common.append(c)


fig, ax = plt.subplots()
ax.grid(True)
ax.plot(st_cruft, c_cruft, label='cruft')
ax.plot(st_common, c_common, label='cruft-common')
ax.plot(st_new, c_new, label='cruft-ng')

ax.set_ylim(0, 615)

fig.autofmt_xdate()

plt.legend()
plt.show()
