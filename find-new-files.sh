#! /bin/bash

find /usr/bin /usr/sbin /bin /sbin /usr/lib -maxdepth 2 -type f -user root | \
	while read f ; do
		if [ -e "./$f" ] ; then
			if ! cmp -s "./$f" "$f" ; then
				echo "$f is different"
			fi
		else
			echo "$f is new"
		fi
	done
