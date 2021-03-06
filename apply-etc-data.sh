#! /bin/bash

find etc | \
while read path ; do
	src="$path"
	dst="/$path"
	if [ -d "$src" ] ; then
		mkdir -p "$dst" || exit $?
	elif [ -f "$src" ] && [ -f "$dst" ] && cmp "$src" "$dst" 2>/dev/null 1>/dev/null ; then
		continue
	else
		cp --remove-destination -a "$src" "$dst" || exit $?
	fi
done
