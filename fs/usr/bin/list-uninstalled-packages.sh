#! /bin/bash

# This script shows packages (according to /usr/src) that are not installed
# (have no /packages/*/Current symlink). It also shows packages that could
# be installed right now via `tpkgs` separately.

find /usr/src -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | \
while read pkg ; do
	if [ -e "/packages/$pkg/Current" ] ; then
		continue
	fi
	if [ -z "$(ls "/packages/$pkg" 2>/dev/null)" ] ; then
		echo "$pkg has no versions!"
	else
		echo "$pkg is not installed, but has versions!"
	fi
done
