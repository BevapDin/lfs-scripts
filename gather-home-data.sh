#! /bin/bash

find \
	$HOME/.icewm | \
while read path ; do
	src="$path"
	dst="home/${path##$HOME/}"
	if [ -d "$src" ] ; then
		mkdir -p "$dst" || exit $?
	elif [ -f "$src" ] && [ -f "$dst" ] && cmp "$src" "$dst" >/dev/null ; then
		continue
	else
		cp --remove-destination -a "$src" "$dst" || exit $?
	fi
done
