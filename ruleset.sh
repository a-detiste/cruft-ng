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
case "$1" in
    "")
        exit 0
        ;;
    UNRELEASED)
        exit 0
        ;;
    unstable)
        exit 0
        ;;
    explain)
        echo "Backport of explain scripts is not supported ATM"
        exit 1
        ;;
    focal)
        release="bullseye"
        ;;
    *)
        release="$1"
esac

if ! [ -d "archive/$release" ]
then
    echo "WARNING: no rule defined for release \"$release\""
    exit 0
fi

(cd "archive/$release"
ls | sort | while read file
do
	echo "$file"
	cat "$file"
done | grep -v ^# | grep .) >> ruleset
