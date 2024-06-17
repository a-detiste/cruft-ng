#!/bin/sh

set -e
set -u
export CRUFT_ROOT=

# all scripts should run without failure or printing to stderr,
# even if the matching package is not installed

OK="\033[1;32mOK\033[0m"
KO="\033[1;31mKO\033[0m"

for script in explain/*
do
   echo -n "$script "
   $script > /dev/null && echo "$OK" || echo "$KO"
done

shellcheck explain/*
