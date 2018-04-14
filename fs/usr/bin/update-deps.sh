#! /bin/bash

file="$1"
if [ "$file" == "--all" ] ; then
	rm /usr/src/*/.needs-pkg
	for d in {/,/usr,/opt}/{lib,bin,sbin}/* ; do
		"$0" "$d"
	done
	exit 0
else
	if ! [ -e "$file" ] ; then
		echo "$file: not found"
		exit 1
	fi
fi

tmpfile="/tmp/$$.tmp"
pkg="$(stat '--format=%U' "$file")"
if ! [ -e "/usr/src/$pkg" ] ; then
	exit 1
fi
needsfile="/usr/src/$pkg/.needs-pkg"

readelf -d "$file" 2>/dev/null | \
	sed '/Shared/ !d; s#^.*\[##;s#\].*$##;s#.*#/lib/&\n/usr/lib/&\n/opt/lib/&#' | \
	xargs -r ls -1 2>/dev/null | \
	xargs -r stat '--format=%U' | \
	sort | \
	uniq > "$tmpfile"

if [ -e "$needsfile" ] ; then
	sort "$tmpfile" "$needsfile" | \
		uniq > "${tmpfile}2" && \
		mv "${tmpfile}2" "$needsfile" && \
		chown "$pkg:" "$needsfile"

	rm "$tmpfile"
else
	mv "$tmpfile" "$needsfile" && \
		chown "$pkg:" "$needsfile"
fi

