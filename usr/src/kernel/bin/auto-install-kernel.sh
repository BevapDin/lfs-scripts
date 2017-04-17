#! /bin/bash

KVERSION="${1?Missing kernel version}"

KSOURCE_TAR="linux-$KVERSION.tar.xz"
KSOURCE_DIR="linux-$KVERSION"

LOCAL_VERSION_SUFFIX="-td"

home="$HOME"

status() {
	echo '================================================='
	echo '================================================='
	echo "$@"
	echo '================================================='
	echo '================================================='
}

status_error() {
	echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	echo "$@"
	echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	echo '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	return 1
}

askyesno() {
	local prompt="${1?Missing prompt}"
	local def="$2"
	while true ; do
		echo -n "$1 [y/n] "
		read answer || exit 1
		if [ -z "$answer" ] ; then
			answer="$def"
		fi
		case "$answer" in
			y|Y)
				return 0
			;;
			n|N)
				return 1
			;;
		esac
	done
}

mvIfNew() {
	local source="${1?Source for mvIfNew is missing}"
	local target="${2?Target for mvIfNew is missing}"
	local file="${source%/*}"
	local tpath="$target/$file"
	if [ "$tpath" = "$source" ] ; then
		return 0
	fi
	if [ -e "$tpath" ] ; then
		if cmp "$tpath" "$source" 1>/dev/null 2>/dev/null ; then
			rm "$source"
			return 0
		fi
		echo "mvIfNew: target exists: $tpath" 1>&2
		return 1
	fi
	mv "$source" "$tpath"
}

# Find a kernel source tar file. The function looks for the source
# archich in several commonly used locatons. If found, the path is printed
# in stdout and the function returns 0. If not found, the function returns 1.
# @param $1 The file name of the source archiv (e.g. linux-4.6.tar.xz)
findtar() {
	local file="$1"
	# Check those dirs for the source
	for d in "$home/" /s/ /s/to-canopus/ /usr/src/kernel/ /mnt/usb/canopus2/lfs/ ; do
		f="$d/$file"
		if [ -e "$f" ] ; then
			echo "$f"
			return 0
		fi
	done
	# Maybe we can reuse a *-ok.tar.xz archiv file? Only if we are not already searching for one.
	if [ "${file%-ok.tar.xz}" != "$file" ] ; then
		return 1
	fi
	fileok="${file%.tar.*}-ok.tar.xz"
	if [ "$fileok" != "$file" ] ; then
		findtar "$fileok" && return 0
	fi
	# Not found
	return 1
}

downloadkernel() {
	local version="$1"
#	local mbv="${version%.*}"
#	local url="ftp://ftp.kernel.org/pub/linux/kernel/v${mbv}/$KSOURCE_TAR"
	local url="https://www.kernel.org/pub/linux/kernel/v4.x/$KSOURCE_TAR"
	if wget -O "$KSOURCE_TAR" "$url" ; then
		return 0
	else
		rm "$KSOURCE_TAR" 2>/dev/null
		return 1
	fi
}

# Go to a common base dir
cd "$home" || exit $?

# Check for source dir, if not existing find the source archiv,
# if this also failes, the archiv is loaded from the internet.
if ! [ -e "$KSOURCE_DIR" ] ; then
	xf="$(findtar "$KSOURCE_TAR")"
	if [ -z "$xf" ] ; then
		echo "Kernel source $KSOURCE_TAR is missing, downloading now..."
		downloadkernel "$KVERSION" || exit $?
	else
		echo "Using source archiv $xf"
		KSOURCE_TAR="$xf"
	fi

	echo "Extracting kernel source $KSOURCE_TAR..."
	tar -xf "$KSOURCE_TAR" || exit $?
	if ! [ -e "$KSOURCE_DIR" ] ; then
		echo "Kernel source $KSOURCE_TAR did not contain the expected folder $KSOURCE_DIR" 1>&2
		exit 1
	fi
fi

echo "Entering kernel source folder $KSOURCE_DIR..."
cd "$KSOURCE_DIR" || exit $?

if ! [ -e ".config" ] ; then
	if [ -e /proc/config.gz ] && askyesno "No config found, shall I use the current config?" y ; then
		echo "Making config from current config..."
		gunzip </proc/config.gz >.config || exit $?
	elif askyesno "Shall I use the config of the previous kernel?" y ; then
		xf="$(ls -1v "$home/linux-"*"/.config" | grep -vF "$KSOURCE_DIR" | tail -1)"
		if [ -z "$xf" ] ; then
			echo "Could not find a previous kernel version" 1>&2
			exit 1
		fi
		cp -v "$xf" .config || exit $?
	else
		echo "No config for kernel, exiting" 1>&2
		exit 1
	fi
fi

echo "Making oldconfig..."
make oldconfig || exit $?
if [ -e /proc/config.gz ] ; then
	echo "Diff to current config is:"
	./scripts/diffconfig <(gunzip </proc/config.gz) .config
fi
while ! askyesno "Is this config OK to you?" ; do
	if askyesno "Do you want to edit the config?" y ; then
		make menuconfig
	else
		exit 1
	fi
done

status 'Making...'
make || exit $?
status 'Installing...'
installkernel || exit $?

status "Making additional modules..."
# Call those scripts to make and install kernel modules
for d in $HOME/bin/make-module-*.sh ; do
	if [ -e "$d" -a -x "$d" ] ; then
		echo "Running $d"
		if ! "$d" "$KVERSION" ; then
			echo "!! $d failed with $?"
		fi
	fi
done

status "Still missing modules:"
list-missing-modules.sh "$(uname -r)" "$KVERSION$LOCAL_VERSION_SUFFIX"

# Tar the source folder
status "Backing up $KSOURCE_DIR [press enter]..."
read
cd .. && mk-ok-tar "$KSOURCE_DIR" || exit $?

# Move the original source away
mvIfNew "$KSOURCE_TAR" /s/to-canopus || exit $?

status "All done!"

exit 0

