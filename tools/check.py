#!/usr/bin/python3

# /usr/share/lintian/data/fields/obsolete-packages

import glob
import os
import subprocess

import apt
import distro_info

STABLE = str(distro_info.DebianDistroInfo().stable())

def check_for_new_release() -> None:
    '''usefull once every two year'''
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

check_for_new_release()

def get_obsolete() -> dict[str, str]:
    obsolete = dict()
    with open('/usr/share/lintian/data/fields/obsolete-packages', 'r') as fd:
         for line in fd:
             line = line.strip()
             if not line:
                 continue
             if line.startswith('#'):
                 continue
             old = line.split()[0]
             try:
                 new_ = line.split(maxsplit=1)[1]
             except IndexError:
                 new_ = ''
             obsolete[old] = new_
    return obsolete

obsolete = get_obsolete()

def get_testing() -> set[str]:
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
    return testing

testing = get_testing()

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
testing.add('king-phisher')
testing.add('powershell-empire')
testing.add('webshells')
testing.add('wordlists')

def get_stable() -> set[str]:
    stable = set()
    with open('tools/Packages_amd64', 'r') as fd:
        for line in fd:
            if line.startswith('Package:'):
                stable.add(line.split(':', 1)[1].strip())
    return stable

stable = get_stable()


filters = set([os.path.basename(f) for f in glob.glob('rules/*') if f == f.lower()])
explain = set([os.path.basename(f) for f in glob.glob('explain/*') if f == f.lower()])

def process(
    category: str,
    packages: set[str],
    testing: set[str],
    stable: set[str],
    obsolete: dict[str, str],
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
    o = packages & obsolete.keys()
    print('obsolete:', sorted(o))

process('rules', filters, testing, stable, obsolete, STABLE)
print()
process('explain', explain, testing, stable, obsolete, 'explain')
