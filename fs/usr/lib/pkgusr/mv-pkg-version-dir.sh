#! /bin/bash

v="$1"
shift
cur="/packages/$USER/Current"
if [ -e "$cur" ] ; then
	cv="$(readlink "$cur")"
	if [ "$cv" != "$v" ] ; then
		echo "Package is currently installed: $cv"
		exit 1
	fi
fi

move-package-version-to-version-dir.sh --override "$USER" / "$v" "$@" | wc -l
ret=${PIPESTATUS[0]}
if [ "$ret" != 0 ] ; then
	echo "ERROR!!"
	echo "ERROR!!"
	echo "ERROR!!"
	exit "$ret"
fi

rm "$cur" 2>/dev/null
ln -s "$v" "$cur"

