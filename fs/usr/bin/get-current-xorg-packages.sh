#! /bin/bash

check_package() {
	local pkg="$1"
	local ver="$2"
	local url="$3"
	local base="/packages/$pkg"
	if [ -e "$base" ] ; then
		if [ -e "$base/$ver" ] ; then
			echo "$pkg is up to date." 1>&2
		else
			echo "$pkg needs to be up dated to: $ver" 1>&2
			echo "install-xorg-packages.sh '$url'"
		fi
	else
		echo "$pkg is not installed." 1>&2
		echo "# install-xorg-packages.sh '$url'"
	fi
}

grep_current_versions() {
	sed 's#-\([0-9.]*\)\.tar\.bz2$# \1#;t;d' | \
	sort -Vsr | \
	sort -k1,1 -us
}

for d in proto app driver font lib ; do
	url="ftp://xorg.freedesktop.org/pub/individual/$d/"
	curl -l "$url" | grep_current_versions | \
	while read pkg ver ; do
		file="$pkg-$ver.tar.bz2"
		pkg="$(echo "$pkg" | tr '[:upper:]' '[:lower:]')"
		check_package "$pkg" "$ver" "$url$file"
	done
done
