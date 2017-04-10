#! /bin/bash

usage() {
	cat >&2 <<EOF
Usage: $0 <old-name> <new-name>
EOF
	exit 1
}

if [ "$#" -ne 2 ] ; then
	usage
fi

oldname="$1"
newname="$2"
newhome="/usr/src/$newname"

if id "$newname" 1>/dev/null 2>/dev/null ; then
	echo "New username $newname already exists!"
	exit 2
fi
if ! id "$oldname" 1>/dev/null 2>/dev/null ; then
	echo "Username $oldname exists not!"
	exit 2
fi
if [ -e "$newhome" ] ; then
	echo "New home dir $newhome exists!"
	exit 2
fi
if [ -e "/packages/$newname" ] ; then
	echo "New package dir /packages/$newname exists!"
	exit 2
fi

usermod -m -d "$newhome" -l "$newname" "$oldname" || exit $?
groupmod -n "$oldname" "$newname" || exit $?
if [ -e "/packages/$oldname" ] ; then
	mv "/packages/$oldname" "/packages/$newname" || exit $?
fi

