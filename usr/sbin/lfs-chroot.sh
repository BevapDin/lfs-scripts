export LFS=/mnt/lfs

if ! mount | grep -q "$LFS" ; then
    mount "$LFS" || exit $?
fi

if ! mount | grep -q "$LFS/dev" ; then
	mount -v --bind /dev $LFS/dev

	mount -vt devpts devpts $LFS/dev/pts -o gid=5,mode=620
	mount -vt proc proc $LFS/proc
	mount -vt sysfs sysfs $LFS/sys
	mount -vt tmpfs tmpfs $LFS/run

	if [ -h $LFS/dev/shm ]; then
		mkdir -pv $LFS/$(readlink $LFS/dev/shm)
	fi
fi

/sbin/chroot "$LFS" /usr/bin/env -i \
    HOME=/root                  \
    TERM="$TERM"                \
    PS1='\u:\w\$ '              \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin \
    /bin/bash --login +h
