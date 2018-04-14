#! /bin/bash

usage() {
cat << EOF
Usage $0 <package-user> <prefix> <package-version>
Find all newly installed files in (see find-new-files-from-user.sh)
and move them into /packages/<package-user>/<package-version>.
This efectifly replaces the prefix of the filepath, e.g.
New installed file "/usr/bin/prog.x" (user=pkg, version=1.0)
With prefix=/usr: moves to /packages/pkg/1.0/bin/prog.x
With prefix=/usr/bin: moves to /packages/pkg/1.0/prog.x
The script creates a link in place of the moved file,
that link points to the new location of the file.

EOF
}

ret=0
override=false

if [ "$#" -lt "3" ] ; then
	usage
	exit 1
fi

while [ "$#" -gt "3" ] ; do
	if [ "$1" == "--override" ] ; then
		shift 1
		override=true
	elif [ "$1" == "--help" -o "$1" == "-?" ] ; then
		usage
		exit 1
	fi
done

user="${1?missing package user}"
prefix="${2?missing prefix}"
version="${3?missing package version}"
shift 3

# erase trailing '/'
prefix="${prefix%/}/"
target="/packages/$user/$version/"
tmp="$(mktemp)"

find-new-files-from-user.sh "$user" > "$tmp"

# Look for files not in prefix: regex is like '^/usr/'
if grep -qve "^${prefix}" < "${tmp}" ; then
	echo "Folowing files are not in $prefix:" 1>&2
	grep -ve "^${prefix}" < "${tmp}" 1>&2
	rm "${tmp}"
	exit 1
fi

mkdir -vp "${target}" || exit $?

# erase leading prefix from file list
sed -i "s#^${prefix}##" "${tmp}"

# Erase filename, leaves the path itself, uniq
# to decrease the number of lines, make the result
# xargs -0 compatible ('\n' --> '\0') and then
# create these folders, but do so in the package
# dir (the input list is relativ to $prefix).
sed 's#/*[^/]*$##' < "${tmp}" | \
	uniq | \
	tr '\n' '\0' | \
	(cd "${target}" && xargs -r -0 mkdir -pv) || exit $?

# useless use of cat, I know
cat "${tmp}" | while read file ; do
	# source
	p="${prefix}${file}"
	# target
	t="${target}${file}"
	if [ -d "${p}" ] ; then
		if [ -e "${t}" ] ; then
			if [ -d "${t}" ] ; then
				continue
			fi
			echo "${t}: is not a folder, should be" 1>&2
			exit 2
		fi
		mkdir -v "${t}" || exit $?
		continue
	fi
	if [ -e "${t}" ] ; then
		if [ -L "${p}" ] ; then
			l="$(readlink "${p}")"
			if [ "$l" == "${t}" ] ; then
				continue
			fi
			echo "${p} links to ${l} not to ${t}" 1>&2
			exit 3
		fi
			
		if cmp -s "${p}" "${t}" ; then
			echo "New file is equal to old file"
			rm "${p}" && ln -v -s "${t}" "${p}" || exit $?
			continue
		fi
		if $override ; then
			echo -e "${t} exists\ndiff '${t}' '${p}'" 1>&2
			exit 3
		fi
		echo "Updating ${p}" 1>&2
	fi

	mv -v "${p}" "${t}" || exit $?
	ln -v -s "${t}" "${p}" || exit $?
done

ret="$?"
rm "$tmp"

exit $ret

