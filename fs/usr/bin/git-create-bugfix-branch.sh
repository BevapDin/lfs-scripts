#! /bin/bash

# This script creates a new git branch (retrieved from upstream/master).
# The name of the new branch can be given as argument, it is automatically created otherwise.

if [ -z "$1" ] ; then
	NAME="$(apg -m 3 -x 3 -n 1 -M l)"
else
	NAME="$1"
fi

git fetch upstream || exit $?
git branch -t "$NAME"  upstream/master || exit $?
git checkout "$NAME"
