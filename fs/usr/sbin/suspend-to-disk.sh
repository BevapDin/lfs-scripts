#! /bin/bash

PATH="$PATH:/sbin:/usr/sbin"

case "$1" in
	shutdown|platform|suspend)
		if pgrep xscreensaver >/dev/null ; then
			xscreensaver-command --lock
		fi
		;;
	*)
		echo "This script takes one parameter: shutdown|platform|suspend!" 1>&2
		exit 1
		;;
esac

echo shutdown >/sys/power/disk || exit $?
echo disk >/sys/power/state || exit $?

exit 0
