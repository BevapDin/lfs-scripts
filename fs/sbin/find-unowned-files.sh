#! /bin/bash

usage() {
cat 1>&2 << EOF
Usage: $0
Find files in / (except /packages, /source, /tmp, /tools)
thar are onwned by root, those should perhaps
be owned by an package user.
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
	-o \( -user root -a -group install \) \
	-o -xtype d \
	-o -user root \
	-print 2>/dev/null

