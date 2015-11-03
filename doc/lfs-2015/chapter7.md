# Chapter 7 of the book

Setting up `/etc` is done by copying existing files from the current LFS system and comparing them to the ones listed in the current version of the book. If need and suitable they are adjusted.

## kernel

To install the kernel, I had to set up the [kernel user scripts](../../usr/src/kernel/bin) in `/usr/src/kernel/bin` ([documentation](../kernel.md)).

Additional (in chroot):
```BASH
# This requires a proper entry in fstab!
mount /boot
# Kind of an install directory:
chown -v root:kernel /boot
chmod -v o+t,g+w /boot
# This is not an install directory.
mkdir -pv /lib/modules
chown -v kernel:root /lib/modules
# Neither is this.
chown -v root:kernel /lib/firmware
chmod -v o+t,g+w /lib/firmware
```

### Explanation

`/boot` is basically an install directory, but instead of having group "install", it has user "kernel". It still has sticky bit. This allows the "kernel" user to install new kernel images there. This is done by the `installkernel` script. Note that other install users (or any other user) don't have write access to `/boot`.

But even the "kernel" user can only add files and delete files that already belong to it. This keeps the files installed for and by `grub` safe (they are owned by root).

Similar `/lib/modules` is *owned* by the "kernel" user and only it may write there. This is fine as there is nothing from other users in it (directly) and therefor nothing needs to be protected or shared.

Same for `/lib/firmware`.

### further installation

Install various scripts from the current LFS system:

[istty](../../usr/bin/istty) - used by `cfcf` to determine whether it is writing to terminal or to a stream.
[mk-ok-tar](../../usr/bin/mk-ok-tar) - make a tar.bz2 archive from a folder and delete the folder.

### firmware

Install teh firmware (if any) from the current LFS system (outside of chroot):
```BASH
cp -v /lib/firmware/dvb-usb-dib0700-* $LFS/lib/firmware
```
