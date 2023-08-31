#!/bin/bash
set -e
rm -f ruleset

function concat() {
(
release="$1"
find "$release" -type f -printf "%f\n" | sort | while read -r file
do
	echo "$file"
	while read -r rule
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
	done < "$release/$file"
done
)
}

concat rules > ruleset

if dpkg-vendor --derives-from Ubuntu
then
    concat ubuntu/devel >> ruleset
else
    concat archive/sid >> ruleset
fi

concat non-free >> ruleset

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

if [ -d "archive/$release" ]
then
    concat "archive/$release" >> ruleset
elif [ -d "ubuntu/$release" ]
then
    concat "ubuntu/$release" >> ruleset
fi

if ! [ "$(readlink archive/stable)" == "$release" ]
then
    concat archive/stable >> ruleset
fi
