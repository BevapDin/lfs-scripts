#! /bin/bash

find \
	/etc/pkgusr \
	/etc/syslog.conf \
	/etc/profile \
	/etc/profile.d/extrapaths.sh \
	/etc/profile.d/dircolors.sh \
	/etc/bashrc \
	/etc/pkgusr \
	/etc/suid-programs | \
while read path ; do
	src="$path"
	dst="fs/$path"
	if [ -d "$src" ] ; then
		mkdir -p "$dst" || exit $?
	elif [ -f "$src" ] && [ -f "$dst" ] && cmp "$src" "$dst" 2>/dev/null 1>/dev/null ; then
		continue
	else
		mkdir -p "${dst%/*}" || exit $?
		cp --remove-destination -a "$src" "$dst" || exit $?
	fi
done
