#! /bin/bash

list_files() {
	find fs/usr/bin fs/bin fs/usr/sbin fs/sbin fs/usr/lib fs/etc -! -type d | \
		sed 's#^fs/#/#'
	if [ -e /tools ] ; then
		find fs/tools -type d | \
			sed 's#^fs/#/#'
	fi
}

list_files | sed 's#/[^/]*$##' | xargs -d '\n' mkdir -p

list_files | while read dst ; do
	src="fs$dst"
	cmp "$src" "$dst" >/dev/null 2>/dev/null && continue
	cp --remove-destination -a "$src" "$dst"
done
