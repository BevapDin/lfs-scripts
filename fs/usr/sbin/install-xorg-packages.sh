#! /bin/bash

usage() {
	cat <<EOF
Installs an XORG package, given by its URL.
Includes download, unpacking, configure, make, make install and make-version
Usage:
$0 [options]* <url> [<package> [<version>]]
This installs a single package loaded from the given URL.
package name and version are automaticly deducted from the URL if not
given.

Or:
$0 --list <listfile> <baseurl>
This greps the file names from the listfile (one filename per line, lines starting
with a '#' are ignored, everything up to the last space is ignored, too.
Example:
somemd5sum libXXX-4.3.2.tar.gz
libXX-4.4.4.tar.bz2
# libOO-4.4.4.tar.bz2
^^ libOO would not be installed, but libXXX would as would libXX)
The script calls itself with the URL formed by appending the file name to
the baseurl parameter (baseurl should end with a '/').

Options:
--ignore-installed: if version is already installed, reinstall it anyway.
Without this option packages that are installed currently are skipped
EOF
	exit 1
}

ignore_installed=false
has_pkg_ver=false

url=""
pkg=""
version=""

while [ "$#" -gt 0 ] ; do
	if [ "$1" = "--list" -a "$#" == 3 ] ; then
		filelist="$2"
		baseurl="$3"
		shift 3
		for pkg in $(sed '/^ *#/d;s#^.* ##' <"$filelist") ; do
			"$0" "$baseurl""$pkg" </dev/null || exit $?
		done
		exit 0
	fi
	if [ -z "$url" ] ; then
		url="$1"
		shift 1
	elif [ -z "$pkg" ] ; then
		pkg="$1"
		shift 1
	elif [ -z "$version" ] ; then
		version="$1"
		shift 1
	else
		case "$1" in
			--ignore-installed)
				ignore_installed=true
				shift 1
				;;
			*)
				echo "Unknown option: $1" 1>&2
				usage
				;;
		esac
	fi
done

if [ -z "$url" ] ; then
	usage
fi

archiv="${url##*/}"
burl="${url%/*}/"
bdir="$archiv"
bdir="${bdir%.tar.bz2}"
bdir="${bdir%.tar.gz}"
bdir="${bdir%.tar.gz}"
if [ -z "$pkg" ] ; then
	pkg="$(echo "$bdir" | sed 's#^\(.*\)-[0-9.]*#\1#' | tr '[:upper:]' '[:lower:]')"
	if [ -z "$pkg" -o "$pkg" == "$bdir" ] ; then
		echo "Missing package name" 1>&2
		exit 1
	fi
fi
if [ -z "$version" ] ; then
	version="$(echo "$bdir" | sed 's#^.*-\([0-9.]*\)$#\1#')"
	if [ -z "$version" -o "$version" == "$bdir" ] ; then
		echo "Missing version" 1>&2
		exit 1
	fi
fi

if [ -e "/packages/$pkg/Current" ] ; then
	if [ "$(readlink "/packages/$pkg/Current")" = "$version" ] ; then
		echo "NOTE: Package $pkg is already installed ($version)."
		exit 0
	fi
	echo "NOTE: Package $pkg seems to be installed: $(readlink "/packages/$pkg/Current")"
fi
if [ -e "/packages/$pkg/$version" ] ; then
	if su "$pkg" -c "tpkgs --bypass --install '$pkg' '$version'" ; then
		echo "NOTE: Package $pkg ($version) existed."
		exit 0
	fi
	echo "NOTE: Package seems to exist: /packages/$pkg/$version"
	if ! $ignore_installed ; then
		exit 0
	fi
fi
if [ -e "/packages/$pkg/Current" ] ; then
	echo "NOTE: Package seems to be installed: $(readlink "/packages/$pkg/Current")"
	echo
fi

tmpfile="$(mktemp)"
cat > "$tmpfile" << EOF
if ! [ -e '.url' ] ; then
	echo '$burl' > '.url'
	echo 'Updated .url file to:'
	echo '$burl'
fi
if ! [ -e '$archiv' ] ; then
	if ! wget -O '$archiv' -o /dev/null '$url' ; then
		rm '$archiv' 2>/dev/null
		echo 'Failed to download $archiv from $url'
		exit 1
	fi
	echo 'Downloaded from $url ...'
	if ! [ -e '$archiv' ] ; then
		echo '... but the expected file $archiv is not there!' 1>&2
		exit 1
	fi
	echo '... to $archiv'
fi
if ! [ -e '$bdir' ] ; then
	if ! tar -xf '$archiv' ; then
		rm '$archiv' 2>/dev/null
		exit 1
	fi
	if ! [ -e '$bdir' ] ; then
		echo '$archiv did not extract to $bdir' 1>&2
		exit 1
	fi
	echo 'Extracted $archiv to $bdir'
fi
cd '$bdir' || exit \$?
if ! cfgc --quiet --xorg --no-install --no-make 1>"\$HOME/conf.1.log" 2>"\$HOME/conf.2.log" </dev/null ; then
	cat "\$HOME/conf.2.log" 1>&2
	exit 1
fi
if ! make 1>"\$HOME/make.1.log" 2>"\$HOME/make.2.log" </dev/null ; then
	cat "\$HOME/make.2.log"
	exit 1
fi
if [ -e '/packages/$pkg/Current' ] ; then
	tpkgs --uninstall || exit \$?
	echo 'Previous version uninstalled'
fi
if ! make install 1>"\$HOME/install.1.log" 2>"\$HOME/install.2.log" </dev/null ; then
	cat "\$HOME/install.2.log"
	exit 1
fi
if ! mk-vers.sh '$version' ; then
	echo "Failed to pack the package (mk-vers.sh failed)" 1>&2
fi
cd ..
if mk-ok-tar '$bdir' ; then
	mv -v "$archiv" /s/to-canopus
	rm {conf,make,install}.{1,2}.log
else
	echo "Failed to tar source dir $archiv" 1>&2
fi

echo 'Everything is fine!'
EOF

if tty 1>/dev/null 2>/dev/null ; then
	cat << EOF
url: $url
archiv (filename only): $archiv
dir (source/build dir): $bdir
pkg (package name): $pkg
ver (version string only): $version
script: $tmpfile
OK? [answer with y]
EOF
	read a || exit $?
	if [ "$a" != 'y' ] ; then
		rm "$tmpfile"
		exit 1
	fi
else
	echo "Installing $pkg ($version)"
fi

cat "$tmpfile" | install_package "$pkg"
rm "$tmpfile"

