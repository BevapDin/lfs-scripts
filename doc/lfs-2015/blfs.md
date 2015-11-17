# BLFS

### perl modules

Use `cpan` from the `perl` install user. This works nice. Note that if `cpan` was started before `/tools` has been removed, it may still have references to binaries there. You will get an error when (after removing `/tools`) you try to install stuff. To fix this, edit `/usr/src/perl/.cpan/CPAN/MyConfig.pm`. Simply replacing `/tools` with `/usr/bin` doesn't work. Some binaries are there, but some are in `/bin`. You need to do this by hand, but there aren't many entries.

### lynx

Make sure to configure with `--with-ssl` as many URLs used to retrieve packages use HTTPS.

Install the [`new`](../../usr/lib/pkgusr/new) script from the current LFS system. It uses the URL from `$HOME/.url` to open `lynx`, so the user can easily download the newest package.

### cmake

Remove the `--cache-file=config.cache` option from `configure`. The ccache part of cmake will not work correctly with it.

### pam

After the installation:
```BASH
mk-install-dir.sh /lib/security
```

### guile

Apply a [patch](../../patches/guile-2.0.11-include-poll.h.patch).

As per instructiuon from the book, a file should be moved to `/usr/share/gdb/auto-load/usr/lib`

### xcb-util

```BASH
mk-install-dir.sh /usr/include/xcb
```

### fontconfig

Create a directory that is otherwise created by `make install`, but won't work because it is not in a install directory:
```BASH
mkdir -pv /usr/src/fontconfig/pkg/answer
chown -v fontconfig:fontconfig /usr/src/fontconfig/pkg/answer
```
Note that the directory is *not* to be made an install directory.

### mesa

```BASH
mk-install-dir.sh /usr/include/GL /usr/include/GL/internal
```

### fam

Needs a [patch](../../patches/fam-2.7.0-buildfixes.patch).

The configure script must be made writable:
```
chmod -v +w configure
```

### gdb

After the installation:
```BASH
mk-install-dir.sh /usr/share/gdb/auto-load/usr/lib
```

### glib

Fix improper installation directory:
```BASH
sed -i 's#ABS_GLIB_RUNTIME_LIBDIR *= *$#ABS_GLIB_RUNTIME_LIBDIR=/usr/lib#' */Makefile
```
This installs the python files to `/usr/share/gdb/auto-load/usr/lib` (which is an install directory) and not to `/usr/share/gdb/auto-load` (which is not writable).

### sudo

Install it as you like. The following configuration is useful for the package manager:

```
# The i-need script may create a new install-user (with
# the script install_package) and/or su to a install user
%install ALL = (root) NOPASSWD:NOSETENV: /usr/lib/pkgusr/i-need
# Install user may install new library files and need to
# activate them using ldconfig
%install ALL = (root) NOPASSWD:NOSETENV: /sbin/ldconfig ""

# The kernel user compiles and installs the kernel
# to /boot, therefor they need to mount it
kernel ALL = (root) NOPASSWD:NOSETENV: /bin/mount /boot
# And also unmount
kernel ALL = (root) NOPASSWD:NOSETENV: /bin/umount /boot
# May need to load the module with the configuration of the current kernel
# to use it as base for the configuration of the new kernel.
kernel ALL = (root) NOPASSWD:NOSETENV: /sbin/modprobe configs
```

Additional for the local user (replace `your_user_account` with the proper user name):
```
# Allow updating the kernel symlinks in /boot, no need to login each time.
your_user_account ALL = (root) NOPASSWD:NOSETENV: /usr/sbin/updatekernel ""
# May su to any install user to maintain this installation.
your_user_account ALL = (%install) NOPASSWD:NOSETENV: /bin/bash
# This is needed for automatic shutdown
your_user_account ALL = (root) NOPASSWD:NOSETENV: /sbin/shutdown
# Need this to create new install users.
your_user_account ALL = (root) NOPASSWD:NOSETENV: /usr/sbin/install_package
```

Install scripts that can be used now:
[i-need](../../usr/lib/pkgusr/i-need) - used by install user when they encounter a dependcy.
[updatekernel](../../usr/sbin/updatekernel) - updates the kernel symlinks in /boot after a new kernel has been tested to work properly.

### Python 2

Some files conflict with the installation of Python 3. The "lazy" solution is to remove / backup them, before installing. `2to3` is actually only a link to `2to3-3.4`. The later can be called when explicitly required, so we can just remove it:
```BASH
rm -v /usr/bin/2to3
```

### smartmontools

Tries to install `/etc/rc.d/init.d/smartd`, which is not allowed and the script isn't useful for us anyway (it does not recognize an LFS system). Use the old version from the current LFS instead, if desired at all.

Don't forget to install symlinks to make it start and be stopped automatically.

```BASH
cp -v /etc/rc.d/init.d/smartd $LFS/etc/rc.d/init.d
for d in /etc/rc.d/rc*/*smartd ; do cp -a -v $d $LFS/$d; done
```

### boinc

Install the start scripts from the current LFS system:
```BASH
cp -v /etc/rc.d/init.d/boinc-client $LFS/etc/rc.d/init.d
for d in /etc/rc.d/rc*/*boinc-client ; do cp -a -v $d $LFS/$d; done
```

### tcl

After the installation:
```BASH
mk-install-dir.sh /usr/share/man/mann/
```

### git

Apply the [patch](../../patches/git-2.5.0-man-pages-installation.patch) to prevent failures when installing the man pages (script tries to change the permissions of existing directories).

### gobject-introspection

After the installation:
```BASH
mk-install-dir.sh /usr/lib/girepository-1.0 /usr/share/gir-1.0
```

### GTK+ 2

The install user is named "gtk-plus-2". It requires "gdk-pixbuf", built *with* "gobject-introspection". If you have build "gdk-pixbuf" before installing "gobject-introspection", you'll have to build and install it again, *after* installing "gobject-introspection".

### dbus

After the installation:
```BASH
mk-install-dir.sh /etc/dbus-1/system.d
mk-install-dir.sh /usr/share/dbus-1/interfaces
mk-install-dir.sh /usr/share/dbus-1/system-services
```

### polkit

After the installation:
```BASH
mk-install-dir.sh /usr/share/polkit-1/actions
```

### pulseaudio

Add `--with-bash-completion-dir=/etc/bash_completion.d` to `configure`.

### dhcpd

Fix the `dhclient-script` (in `/sbin`) to *not* override `/etc/resolv.conf`. Use the static file from the current LFS system instead.

### apache

Move the new configuration out of the way (`mv /etc/apache{,.original}`), copy the old configuration and adjust as needed.

Errors that might come up:
- "could not open mime types config file" - copy `/etc/mime.types` from the current system.
- "Name or service not known: AH01564" - make sure `named` is up and running and is configured with the proper domain, also make sure the `/etc/resolv.conf` is correct (not overridden by `dhclient`)
- "Failed to create shared memory" - edit `/etc/sysconfig/createfiles`, add entries for the shared memory files.

TODO: fix this directory to point to /tmp

### KDE

(Note: this is from an older installation and not tested. It's here so it's not forgotten.)
```BASH
# needs to be owned by root for security
chown 0 /opt/kde4/lib/kde4/libexec/fileshareset
chown 0 /opt/kde4/lib/kde4/libexec/kcheckpass
chown 0 /opt/kde4/lib/kde4/libexec/start_kdeinit
chown 0 /opt/kde4/lib/kde4/libexec/kdesud
# needed for login
chmod u+s /opt/kde4/lib/kde4/libexec/start_kdeinit
chmod u+s /opt/kde4/lib/kde4/libexec/fileshareset
```
