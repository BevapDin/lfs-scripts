#! /bin/bash

#driverdir="$HOME/ati-driver-11-9"
#driverdir="$HOME/amd-driver-12-4"
driverdir="$HOME/amd-driver-12-6"
#driverdir="$HOME/amd-driver-12-10"
destname="fglrx.ko"

if [ -n "$1" ] ; then
	ver="$1"
else
	ver="$(uname -r)"
	ver="${ver/linux-}"
fi

kdir="/lib/modules/$ver"
if ! [ -e "$kdir" ] ; then
	if [ -e "$kdir-tk" ] ; then
		$0 "$ver-tk"
		exit $?
	fi
	echo "Kernel is not installed into $kdir" 1>&2
	exit 1
fi

destdir="$kdir/kernel/misc"

if ! [ -d "$destdir" ] ; then
	sudo /usr/sbin/mk-install-dir.sh "$destdir" || exit $?
fi

if ! [ -d "$destdir" ] ; then
	echo "Dest dir ($destdir) does not exist" 1>&2
	exit 1
fi

cd "$driverdir/common/lib/modules/fglrx/build_mod/" || exit $?
rm 2.6.x/*.o "../fglrx.$ver.ko" 2>/dev/null

./make.sh --norootcheck "--uname_r=$ver" || exit $?
cp "../fglrx.$ver.ko" "$destdir/$destname" || exit $?
if sudo /sbin/depmod "$ver" ; then
	echo "Sudo depmod succeeded!"
else
	echo "Don't forget to run 'depmod $ver'"
fi
exit 0

