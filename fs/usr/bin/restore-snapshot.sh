#! /bin/bash

# This script reads a list of package and version pairs and tries to install
# those versions via `tpkgs`.
# The input is one line per package in the format '<pkg>/<version>'.
# Lines starting with '#' are ignored.
# If the package version is already installed, it is ignored.
# Options:
# '--dry-run' - the script only prints what it would do.

# It's is supposed to be used with the output of `make-snapshot.sh`.

if [ "$1" = '--dry-run' ] ; then
	tpkgs() {
		echo "tpkgs $@"
	}
fi

IFS=/
grep -vE '^ *#' | \
while read pkg version ; do
	c="/packages/$pkg/Current"
	if [ -e "$c" ] && [ "$(readlink "$c")" = "$version" ] ; then
		continue
	fi
	tpkgs --bypass --install $pkg $version
done
