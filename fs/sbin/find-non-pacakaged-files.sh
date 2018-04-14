#! /bin/bash

usage() {
cat 1>&2 << EOF
Usage: $0
Find files in / (except /packages, /source, /tmp, /tools)
that are not links into a package dir (into /packages)
EOF
exit 1
}

if [ "$#" != 0 ] ; then
	usage
	exit 1
fi

find -P / \
	-xdev \
	-path /packages -prune \
	-o -path /sources -prune \
	-o -path /tools -prune \
	-o -path /tmp -prune \
	-o -user root \
	-o -group install \
	-o -type f \
	-print 2>/dev/null

