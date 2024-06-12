#!/bin/bash
LC_ALL=C.UTF-8

set -e

RULESET="$1"
shift
rm -f "$RULESET"

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
			"# END "*"$RULESET"*)
			    break
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

concat rules > "$RULESET"

concat non-free >> "$RULESET"

# releases are in reverse order
if dpkg-vendor --is kali
then
    concat kali >> "$RULESET"
    exit 0
elif dpkg-vendor --derives-from Ubuntu
then
    concat ubuntu/devel >> "$RULESET"
    archive="ubuntu"
    releases="jammy focal bionic xenial"
else
    concat archive/sid >> "$RULESET"
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
        concat "archive/$r" >> "$RULESET"
    fi
    if [ "$r" = "$release" ]
    then
        # we are done
        exit 0
    fi
done
