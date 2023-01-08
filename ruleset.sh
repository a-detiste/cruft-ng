#!/bin/bash
set -e
rm -f ruleset

function concat() {
(cd $1
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

done | grep -v ^# | grep .
)
}

concat rules > ruleset

# backport
release="$1"
case "$release" in
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
    *-backports)
        release="${release%\-backports}"
        ;;
esac
echo "release:$release"

if ! [ -d "archive/$release" ]
then
    echo "WARNING: no rule defined for release \"$release\""
    exit 0
fi

if ! [ "$(readlink archive/stable)" == "$release" ]
then
    concat archive/stable >> ruleset
fi

concat archive/$release >> ruleset
