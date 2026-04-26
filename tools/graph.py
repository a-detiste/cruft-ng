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

# split cruft <-> cruft-common
SEED: dict[datetime, int] = {
    datetime(1998, 4, 10): 81, # https://lists.debian.org/debian-policy/1998/04/msg00089.html
}

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
