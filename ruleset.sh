#!/bin/bash
LC_ALL=C.UTF-8

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
			'#'*)
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

concat non-free >> ruleset

# releases are in reverse order
if dpkg-vendor --is kali
then
    concat kali >> ruleset
    exit 0
elif dpkg-vendor --derives-from Ubuntu
then
    concat ubuntu/devel >> ruleset
    archive="ubuntu"
    releases="jammy focal bionic xenial"
else
    concat archive/sid >> ruleset
    archive="archive" # 'debian/' has a special meaning
    releases="forky trixie bookworm bullseye buster"
fi

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
    *-backports)
        release="${release%\-backports}"
        ;;
esac
echo "target release: $release"

for r in $releases
do
    if [ -d "$archive/$r" ]
    then
        echo "adding: $r"
        concat "archive/$r" >> ruleset
    fi
    if [ "$r" = "$release" ]
    then
        # we are done
        exit 0
    fi
done
