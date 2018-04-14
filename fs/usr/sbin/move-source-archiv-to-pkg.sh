#! /bin/bash

usage() {
	cat >&2 <<EOF
Usage: $0 <pkg-user> <archive-files>

Or to move all new files for a packages from /s/: $0 --all

EOF
	exit 1
}

doAll() {
	cd /usr/src || exit $?
	for user in * ; do
		# Find files whose names start with the username and
		# have and have some non-alphanumeric behind
		# This finds files like "libab-4.1.tar"
		# but not files like "libabc-4.1.tar"
		# Than call this script this time with user name and files
		# to move.
		ls -1 /s/ | \
			grep -iEe "^${user}[^a-z0-9].*" | \
			xargs -d '\n' -r "$0" "$user"
		# Additionaly print files that start with the user.
		# These files have either not been greped in the above,
		# or (unlikley) have not been moved successfully.
		# This is for informational usage only.
		if ls -1 /s/ | grep -iqEe "^${user}" ; then
			echo "Not sure if these are for $user:"
			ls -1 /s/ | grep -iEe "^${user}"
		fi
	done
	find . -maxdepth 2 -type d | while read path ; do
		file="${path#*/}"
		find /s/ -maxdepth 1 -type f -iname "$file*" | while read op ; do
			echo "What about $op <= $path"
		done
	done
	exit 0
}

# Command line is: <pkg-user> <files>*
user="$1"
shift 

if [ "$user" = "--all" ] ; then
	doAll
	exit 0
fi

# No files given or no user given
if [ "$#" = 0 -o -z "$user" ] ; then
	usage
fi
# Hack: if only one file has been given, check
# if the user swapped file and user on the command line,
# in other words: check if the command line was
# actually: <file> <pkg-user>
if [ "$#" = 1 ] ; then
	if ! [ -e "/usr/src/$user" ] ; then
		# So the user name is invalid (has no home dir in /usr/src),
		# swap $user an $1
		x="$user"
		user="$1"
		shift 1
		set -- "$x"
	fi
fi

cd /s || exit $?

chown -v "$user:$user" "$@" || exit $?
mv -vb "$@" "/usr/src/$user/" || exit $?

exit 0

