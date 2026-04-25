#!/usr/bin/python3

import datetime
import os

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as mtick

from git import Repo

def graph(package: str, rules: str) -> tuple[list[datetime.datetime],list[int]]:
    repo = Repo.init('/home/tchet/deb/%s' % package)
    count = dict()

    for c in repo.iter_commits(all=True, paths=[rules]):
        blob = c.tree[rules]
        count[c.authored_date] = len(blob.list_traverse())

    stamps = []
    values = []
    for k, v in sorted(count.items()):
        stamps.append(datetime.datetime.fromtimestamp(k))
        values.append(v)
    return stamps, values

st_old, c_old = graph('cruft', 'filters-unex')
st_new, c_new = graph('cruft-ng', 'rules')

fig, ax = plt.subplots()
ax.grid(True)
ax.plot(st_old, c_old, label='cruft')
ax.plot(st_new, c_new, label='cruft-ng')

ax.set_ylim(0, 615)

fig.autofmt_xdate()

plt.legend()
plt.show()
