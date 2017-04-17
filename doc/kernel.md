# The kernel scripts
====

Used to build/update/install the linux kernel.

Building and installing the kernel is a bit different than normal packages.

### installkernel

This is a wrapper around
```BASH
make modules_install
cp "vmlinuz" "/boot/lfskernel-$VERSION"
cp System.map "/boot/System.map-$VERSION"
cp .config "/boot/conifg-$VERSION"
cp -r Documentation/* "/usr/share/doc/linux-$VERSION"
```

In simple words: it installs the modules, the kernel to `/boot` (including a copy of the configuration and the `System.map`) and the documentation to `/usr/share/`.

The boot manager (`grub` usually) has options to boot from
- `/boot/vmlinuz.old`,
- `/boot/vmlinuz` (this one is the default), and
- `/boot/vmlinuz.new` (`installkernel` makes this a symbolic link to the installed kernel).

After installing a kernel I can reboot, select the third entry (`vmlinuz.new`) and see if it works. If it does not work, the next boot will use the default (`vmlinunz`) or I can even go back to the previous kernel that works for sure (`vmlinuz.old`). Once I'm convinced that the new kernel is fine, I call `updatekernel` which renames `vmlinuz.new` to `vmlinuz` (and the previous one to `vmlinuz.old`). The default boot goes to the new kernel than.

Those scripts take care of backups (the files in `/boot`), mounting `/boot`, using the kernel version (detected by looking in `include/config/kernel.release`) and of course errors.

### listkernels

List the kernels in `/boot` and which one is currently used and which is the default and which is the current.

### list-missing-modules.sh

Takes two installed kernel versions as parameters and lists the modules that one of them has and the other does not.
It does basically a `find /lib/modules/$VERSION | sort` of both versions and runs `diff -y` on both streams.

Normally the output should be empty as the new kernel has the same modules as the old one. If there is output than either modules have been enabled or disabled as compared to the old kernel. This may indicate a problem with the kernel configuration. It's usually worth an quick inspections.

### update-grub-config

For each new kernel a separate entry is added to the boot manager (`grub`) configuration. This script does this. Each entry contains the version of the kernel. This allows to boot a specific kernel version (as opposed to just new/current/old).

### auto-install-kernel.sh

Given a kernel version this script
- downloads the kernel source (if not already present),
- unpacks it,
- configures it using `make oldconfig`,
- compiles it,
- installs it.

Installing a new kernel is as easy as `auto-install-kernel.sh 3.10.4`

### make-module-ati-driver.sh

All scripts that match "$HOME/bin/make-module-*.sh" are automatically run by `auto-install-kernel.sh`. This makes the ati display driver under the appropriate install user.

## sudo

Some of the scripts require root privileges. This is usually done via `sudo`, which requires proper entries in `/etc/sudoers`.

## sudo

Some of the scripts require root privileges. This is usually done via `sudo`, which requires proper entries in `/etc/sudoers`.

Example: mounting `/boot`, running `updatekernel` as normal user, installing additional modules as install user.
