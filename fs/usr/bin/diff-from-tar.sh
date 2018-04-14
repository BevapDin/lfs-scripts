#! /bin/bash

usage() {
	echo "Usage: $0 <tar-file> <dir>"
}


if [ "$#" != "2" ] ; then
	usage
	exit 1
fi


tmpdir="$(mktemp)"
if [ -z "$tmpdir" ] ; then
	echo "mktemp failed!" 1>&2
	exit 1
fi
rm "$tmpdir" || exit $?
mkdir "$tmpdir" || exit "$?"

doit() {
	local tarfile="$1"
	local dir="$2"

	cd "$tmpdir" || return $?
	tar -xf "$tarfile" || return $?
	find . -type f | while read file ; do
		if ! [ -e "$dir/$file" ] ; then
			echo "Missing: $file"
			continue
		fi
		diff -u "$file" "$dir/$file"
	done
}

doit "$@"

rm -Rf "$tmpdir"

