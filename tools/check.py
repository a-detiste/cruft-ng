#!/usr/bin/python3

# /usr/share/lintian/data/fields/obsolete-packages

import glob
import os
import subprocess

import apt
import distro_info

STABLE = str(distro_info.DebianDistroInfo().stable())

testing = set()
c = apt.Cache()
for p in c.keys():
    o = c[p]
    if o.candidate:
        d = o.candidate.record['Description'].lower()
        if 'transitional' in d or 'dummy' in d:
            pass
        else:
            testing.add(o.shortname)
testing.add('usr-is-merged')

# Add RaspBian
testing.add('raspberrypi-bootloader')
testing.add('libraspberrypi0')

# Add BeagleBone
testing.add('bb-customizations')
testing.add('bb-node-red-installer')
testing.add('c9-core-installer')
testing.add('doc-beaglebone-getting-started')
testing.add('librobotcontrol')
testing.add('ti-pru-cgt-installer')

# Add Hurd
testing.add('hurd')
testing.add('libc0.3')

# Add Kali Linux
testing.add('kali-archive-keyring')
testing.add('kali-defaults')
testing.add('kali-menu')
testing.add('kali-themes')
testing.add('kali-themes-common')
testing.add('kali-undercover')
testing.add('king-phisher')
testing.add('powershell-empire')
testing.add('webshells')
testing.add('wordlists')

old_stable = os.path.basename(os.readlink('archive/stable'))
if old_stable != STABLE:
    print('Stable distribution has changed: %s -> %s' % (old_stable, STABLE))
    os.unlink('archive/stable')
    os.chdir('archive')
    if not os.path.isdir(STABLE):
        os.mkdir(STABLE)
        gitkeep = os.path.join(STABLE, '.gitkeep')
        open(gitkeep, 'w').close()
        subprocess.call(['git', 'add', gitkeep])
    os.symlink(STABLE, 'stable')
    os.chdir('..')

if old_stable != STABLE or not os.path.isfile('tools/Packages_amd64'):
    subprocess.check_call(['ben', 'download',
                           '--archs', 'amd64',
                           '--suite', 'stable'],
                          cwd='tools')
    os.unlink('tools/Sources')

stable = set()
with open('tools/Packages_amd64', 'r', encoding='utf8') as fd:
    for line in fd:
        if line.startswith('Package:'):
            stable.add(line.split(':', 1)[1].strip())


filters = set([os.path.basename(f) for f in glob.glob('rules/*') if f == f.lower()])
explain = set([os.path.basename(f) for f in glob.glob('explain/*') if f == f.lower()])

def process(
    category: str,
    packages: set[str],
    testing: set[str],
    stable: set[str],
    archive: str
) -> None:
    unknown = packages - testing
    deprecated = unknown & stable
    unknown = unknown - stable
    print('[%s]:' % category)
    print('moved to %s:' % STABLE, sorted(deprecated))
    for item in deprecated:
        destdir = os.path.join('archive', archive)
        if not os.path.isdir(destdir):
            os.makedirs(destdir)
        subprocess.call(['git', 'mv', os.path.join(category, item), destdir])
        gitkeep = os.path.join(destdir, '.gitkeep')
        if os.path.isfile(gitkeep):
            subprocess.call(['git', 'rm', gitkeep])
    print('unknown:', sorted(unknown))

process('rules', filters, testing, stable, STABLE)
print()
process('explain', explain, testing, stable, 'explain')
