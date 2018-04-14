#! /bin/bash

cd /home/thomas || exit $?

for d in $(ls /usr/src) ; do
	newurl="$d.url"
	oldurl="/usr/src/$d/.url"
	if [ -e "$oldurl" ] ; then
		if [ -s "$newurl" ] ; then
			if diff -w "$newurl" "$oldurl" >/dev/null ; then
				rm "$newurl"
			else
				cat "$newurl" "$oldurl"
			fi
		fi
		continue
	fi
	if [ "$UID" = 0 ] ; then
		if [ -s "$newurl" ] ; then
			mv -v "$newurl" "$oldurl" && \
				chown -v "$d:" "$oldurl"
		fi
		continue
	fi
	if [ -s "$newurl" ] ; then
		continue
	fi
	opera "http://freecode.com/search?q=$d"
	head -1 >"$newurl"
done
