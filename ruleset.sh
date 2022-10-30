#!/bin/sh
set -e
rm -f ruleset

(cd rules
ls | sort | while read file
do
	echo "$file"
	cat "$file"
done | grep -v ^# | grep .) > ruleset

# backport
if [ -n "$1" ]
then
    (cd "archive/$1"
    ls | sort | while read file
    do
	echo "$file"
	cat "$file"
    done | grep -v ^# | grep .) >> ruleset
fi
