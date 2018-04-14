#! /bin/bash

# Get the current version of the current kernel.
# Ouputs a list of versions (each on a separate line).

wget --no-check-certificate -O - -o /dev/null "http://www.kernel.org/" | \
	sed '
		s#^.*>\(3\.[0-9][0-9]*\.[0-9][0-9]*\)<.*$#\1#
		t
		d
	' | \
	sort -Vr | \
	uniq
