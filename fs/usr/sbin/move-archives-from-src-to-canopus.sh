#! /bin/bash

case "$1" in
	-h|-\?|--help)
		echo "Move source archives from /s/to-canopus to canopus2 or only list them"
		exit 1
esac

domvx() {
	local file="$1"
	local target="$2/${file##*/}"
	if [ "$file" = "$target" ] ; then
		return 1
	fi
	if [ -e "$target" ] ; then
		if cmp "$target" "$file" ; then
			rm -v "$file"
			return $?
		else
			echo "> $file exists on canopus!" 1>&2
			return 1
		fi
	else
		mv -v "$file" "$target"
		return $?
	fi
}

domv() {
	local file="$1"
	
	if domvx "$file" "/mnt/usb/canopus2/lfs" ; then
		return 0
	fi
	if domvx "$file" "/s/to-canopus" ; then
		return 0
	fi
	return 1
}

# Get all the ok-tars (made by mk-ok-tar)
tmpfile="$(mktemp)"
ls -1 /usr/src/*/*-ok.tar.bz2 > "$tmpfile" || exit $?

# translate the ok-tar name back into the possible original archiv
# names.
sed -n '
	s#-ok\.tar\.bz2$##			# simple: remove ok.tar.bz2
	h;s#$#.tar.bz2#;p			# add tar.bz2 and print
	g;s#$#.tgz#;p				# add tgz
	g;s#$#.tar.gz#;p			# tar.gz
	g;s#$#.tar.xz#;p
	g;s#$#-src.tar.bz2#;p		# add -src and same as above
	g;s#$#-src.tgz#;p
	g;s#$#-src.tar.gz#;p
	g;s#$#-src.tar.xz#;p
	g;s#$#.src.tar.bz2#;p
	g;s#$#.src.tgz#;p
	g;s#$#.src.tar.gz#;p
	g;s#$#.src.tar.xz#;p
' < "$tmpfile" | \
	while read file ; do
		if ! [ -e "$file" ] ; then
			continue
		fi
		if [ -d "$file" ] ; then
			echo "Dir: $file"
			continue
		fi
		domv "$file"
	done

rm "$tmpfile"

if [ -e /mnt/usb/canopus2/lfs ] ; then
	find /s/to-canopus/ -maxdepth 1 -type f | \
		while read file ; do
			domv "$file"
		done
fi

