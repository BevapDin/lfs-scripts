#! /bin/bash

list_new_pkgs=false
list_cur_pkgs=false
install_now=false

read_http_dir() {
	local group="$1"
	local murl="http://xorg.freedesktop.org/releases/individual/$group/"
	lynx -dump -listonly "$murl" | grep -E '\.tar\.bz2$' | sed 's#^[ 0-9]*\. ##'
}

parse_http_links() {
	sed 's#^.*/\([^/]*\)/\([^/]*\)-\([0-9.]*\)\.tar\.bz2$#\1 \2 \3#' | \
	sort -Vr | \
	sort -k 1,2 -u
}

check_pkg_ver() {
	local group="$1"
	local pkg="$2"
	local ver="$3"
	local url="http://xorg.freedesktop.org/releases/individual/$group/$pkg-$ver.tar.bz2"
	pkg="$(echo "$pkg" | tr '[:upper:]' '[:lower:]')"
	local pdir="/packages/$pkg"
	local vdir="$pdir/$ver"
	if ! [ -e "$pdir" ] ; then
		if $list_new_pkgs ; then
			echo "New packages: $pkg ($group)"
			echo "$url"
		fi
		return 0
	fi
	if ! [ -e "$vdir" ] ; then
		echo "Current version not installed: $pkg ($ver)"
		echo "$url"
		if $install_now ; then
#			install-xorg-packages.sh "$url" <&4
			install-xorg-packages.sh "$url" "$pkg" "$ver"
		fi
		return 0
	fi
	if [ -e "$pdir/Current" ] ; then
		cver="$(readlink "$pdir/Current")"
		if [ "$cver" = "$ver" ] ; then
			if $list_cur_pkgs ; then
				echo "Current: $pkg ($ver)"
			fi
			return 0
		fi
		echo "Not currently activated: $pkg ($ver)"
		return 0
	fi
	echo "$pkg - $ver: unknown"
}

doGroup() {
	local group="$1"
	read_http_dir "$group" | parse_http_links | \
	while read group_ pkg ver ; do
		check_pkg_ver "$group_" "$pkg" "$ver"
	done
}

usage() {
	echo "Usage: $0 [options]* [groups]*"
	echo "Options:"
	echo "--list-new: also list packages that are new (not installed at all)"
	echo "--list-cur: also list versions that are currently installed"
	echo "--install-now: install new versions now, does not isntall new packages."
	exit 1
}

while [ "$#" -gt 0 ] ; do
	case "$1" in
		--list-new)
			list_new_pkgs=true
			shift 1
			;;
		--list-cur)
			list_cur_pkgs=true
			shift 1
			;;
		--install-now)
			install_now=true
			shift 1
			;;
		-?|-h|--help)
			usage
			;;
		-*)
			echo "Unknown option: $1" 1>&2
			exit 1
			;;
		*)
			break
			;;
	esac
done

exec 4<&0

if [ "$#" = 0 ] ; then
	for d in app data doc driver font lib proto util xcb xserver ; do
		doGroup "$d"
	done
else
	for d in "$@" ; do
		doGroup "$d"
	done
fi

