# Chapter 1 - 4 of the book

## filesystem

Creating the filesystems:
```BASH
# /boot
mkfs.ext3 -v /dev/sdb1

# root partition
mkfs -v -t ext4 /dev/sdb3

# Mount point for the root partition in the host system
mkdir -vp /mnt/lfs
```

Entry in fstab (host):
```
# New (as of end of 2015) LFS partition:
/dev/sdb3 /mnt/lfs ext4 noauto,exec,atime,suid,rw 1 0
```

## directoryies

Create $LFS/sources as per book. Download packages as per book.

## lfs user

Set up the lfs user as per book.

Add script `/usr/sbin/lfs.sh` (in host) for switching to the lfs user:
```BASH
export LFS=/mnt/lfs

if ! mount | grep -q "$LFS" ; then
    mount "$LFS" || exit $?
fi

su - lfs
```

Entry in sudoers (in host), add via `visudo`:
```
your_user_account ALL = (root) NOPASSWD: /usr/sbin/lfs.sh
```

This allows to change to the lfs user via `sudo /usr/sbin/lfs.sh`

Create `.bashrc` and `.bash_profile` as per book.

Add this to `.bashrc` (for faster builds, adjust the number as you see fit):
```
export MAKEFLAGS='-j 2'
```

Continue with [chapter 5](chapter5.md).
