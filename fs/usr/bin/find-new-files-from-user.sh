#! /bin/bash

# 1. Only on this drive (/)
# 2. ignore anything in /packages
# 3. ignore anything in /sources
# 4. ignore anything in /tools (overhead from LFS installation)
# 5. ignore anything in /tmp
# 6. ignore any links
# 7. ignore any files from other users

user="${1-$USER}"

#cat /etc/install-dirs | \
#xargs -0 -i \
#find -P '{}' \

find -P / \
	-xdev \
	-path /packages -prune \
	-o -path /sources -prune \
	-o -path /lib/modules -prune \
	-o -path /tools -prune \
	-o -path /tmp -prune \
	-o -type l \
	-o ! -user "$user" \
	-o -print 2>/dev/null

