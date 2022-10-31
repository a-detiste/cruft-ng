#!/bin/bash
set -e
rm -f ruleset

(cd rules
ls | sort | while read file
do
	echo "$file"
	# this is a temporary comptability layer
	# so rules can already be rewriten in new format
	# expected by dh-cruft
	# --- o<  --- o< ---
	while read rule
	do
		case "$rule" in
			"")
			;;
			"#*")
			;;
			*/)
				echo "${rule:0: -1}"
				echo "${rule}**"
			;;
			*)
				echo "$rule"
			;;
		esac
	done < "$file"
	# --- o<  --- o< ---

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
