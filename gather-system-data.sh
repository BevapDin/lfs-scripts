#! /bin/bash

# Find files that should be considered to be put into the repository.

find \
	/usr/bin \
	/usr/sbin \
	/bin \
	/sbin \
	/usr/lib \
	-maxdepth 2 \
	-type f \
	-\( -user root -o -user $USER -\) | \
	while read f ; do
		src="$f"
		dst="fs$f"
		if [ -e "$dst" ] ; then
			if cmp -s "$src" "$dst" ; then
				continue
			fi
		fi
		mkdir -p "${dst%/*}"
		cp "$src" "$dst"
	done
