#! /bin/bash

# This script lists the installed version of each package.
# The output format is '<pkg>/<version>\n' for each package that is
# installed. Packages that are not installed are ignored.
# The output is suitable as input to `restore-snapshot.sh`.
# It can also be stored as text file and it can be edited by hand.

find /packages -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | \
while read pkg ; do
	c="$pkg/Current"
    if [ -e "$c" ] ; then
		v="$(readlink "$c")"
        echo "$pkg/$v"
    fi
done
