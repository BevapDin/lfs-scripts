# Chapter 6 of the book

## Preparing for `chroot`

Put the call to `chroot` command into a script `/usr/sbin/lfs-chroot.sh` (in the host system):

```BASH
# You may want to adjust this.
export LFS=/mnt/lfs

if ! mount | grep -q "$LFS" ; then
    mount "$LFS" || exit $?
fi

# Copy of the script from book (replace with current version from the book):
chroot "$LFS" /tools/bin/env -i \
    HOME=/root                  \
    TERM="$TERM"                \
    PS1='\u:\w\$ '              \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/tools/bin \
    /tools/bin/bash --login +h
```

Add it to `sudoers`:
```
your_user_account ALL = (root) NOPASSWD: /usr/sbin/lfs-chroot.sh
```

## chroot

Enter the `chroot` stage according to ~~the prophecy~~ the book. Continue right before installing the first package. The book creates `/usr/libexec`, but I'll use `/usr/lib` instead, therefor remove `/usr/libexec`:
```BASH
rmdir -v /usr/libexec
```
Note that this requires to pass `--libexecdir=/usr/lib` to the configure script. This is done automatically by `cfgc`.

Make a link for convenience:
```BASH
ln -s /sources /s
```

## Package manager installation

### /etc

According to the hint you should now install the package manager files. Install the stuff that goes into `/etc` according to the hint:
```BASH
cp -a /tools/more_control_helpers/etc /etc/pkgusr
```

However, I don't use some of the files there and therefor I remove them:
```BASH
rm -v /etc/pkgusr/skel-package/{.project,build,build.conf} /etc/pkgusr/build
```

Add some helpfull stuff to /etc/pkguser/bash_profile
```BASH
# This is good for bugs in make/gcc/...
# because error messages are in default language that
# we can easyly look up in the web.
export LC_ALL=C
unset LANG

if [ -e "$HOME/notes" ] ; then
    echo "! There are notes for this package: less -S $HOME/notes"
fi
```
TODO: make that a patch.

### /usr/bin

Don't install the stuff into `/bin`, I don't need it.

### /usr/lib/pkguser

Don't install the stuff in `/usr/lib/pkgusr`, install it from the current LFS system instead:
```BASH
cp -v /usr/lib/pkguser/{chgrp,chmod,chown,install,mkdir,exec} $LFS/usr/lib/pkgusr
```
This must be done from outside the `chroot`.

```BASH
cp -v /usr/lib/pkguser/{cfgc,tpkgs} $LFS/usr/lib/pkgusr
```

TODO: fix the `install` script wrapper to use DAISY_CHAIN

### /usr/sbin

Instead of installing the stuff in `/sbin` according to the hint, you can just move the two relevant scripts `add_package_user` and `install_package` to `/usr/sbin`. The end result is the same, but those scripts are the adjusted one from the current LFS system.
```BASH
mv -v /tools/bin/{add_package_user,install_package} /usr/sbin
```

### install group

Add the install group as per the hint:
```BASH
groupadd -g 9999 install
```

### initial install directories

Install the wrapper script `mk-install-dir.sh` to create install directory with one command.
```BASH
cp -v /usr/sbin/mk-install-dir.sh $LFS/usr/sbin
```
(Again: done from the host system).

Initialize the install dirs. You can use `mk-install-dir.sh` for this:
```BASH
mk-install-dir.sh </tools/more_control_helpers/installdirs.lst
```

TODO: this needs to be added to the git repo

### /packages

At this time you want to create the `/packages` directory. Also make it an install-dir (but don't add it to `/etc/install-dirs`):
```BASH
mkdir -pv /packages
chown -v 0:9999 /packages # adjust for your install group id
chmod +t /packages
```

### nearly ready

Now you're ready to install packages.

Note: at this point `vim` is not installed and `cfgc` can't use it to edit the configuration. You have to edit the `myconfig` file from the host system. You probably want to run `cfcf` with the `--no-configure` switch first to create `myconfig`, edit it externally, run `cfgc` again with the `--no-install` switch so you can run the test suit before installing.

I recommend having two shells open: one for the stuff inside the `chroot` environment, the user inside the host system with current directory being `$LFS/usr/src`. This allows you to edit the configuration (from the second shell) via `vim <pkg-user>/.<build-folder>.config` - usually writing the first few letters are enough, the remaining parts can be filled by bash completion.

The installation is than separated into
- running `cfgc --no-configure` (which creates the `myconfig` file),
- edition the configuration (from the second shell),
- running `cfgc --no-install` (prevent installation to allow running the test suit).

## package specific notes

### common note

Be aware: the book assumes one is in a sub folder of `/source` and paths (e.g. for patches) are relative to it. But for us, the paths must point to `/s`.

### more install directories

Many packages install translations in `/usr/share/locale/<lang>/LC_MESSAGES`. This leads to the problem that one package installs a language folder at first, all other packages that want to install data into that language folder will now fail to install.

You can either wait for this to happen in each package (`make install` will fail).

Or you can run this command after each package has been installed:
```BASH
find /usr/share/locale/ -\( -name LC_MESSAGES -o -name LC_TIME -\) -! -user root|mk-install-dir.sh
```

Additional:
```BASH
mk-install-dir.sh /usr/lib/pkgconfig
```

### glibc

Set up `/var/cache/nscd`, `/etc/localtime as` and `ld.so.conf` as root.
Be aware the the timezone installation entries to extract from `../tzdata2015f.tar.gz` but the file is in `/s`, so adjust that path.

You need to make `/usr/include/scsi` an install directory:
```BASH
mk-install-dir.sh /usr/include/scsi
```

### zlib

Editing `myconfig` is required, it uses a non-standard format and at least the `--prefix` switch has to be added.

### binutils

It wants to install a few translation files. The directories already exists, but are owned by "glibc" and need to be adjusted:
```BASH
mk-install-dir.sh /usr/share/locale/*/LC_MESSAGES
```

### gcc
Do as the hint tells you to (as root):
```BASH
chown -h gcc: /usr/lib/{libgcc_s.so,libgcc_s.so.1,libstdc++.la,libstdc++.so,libstdc++.so.6}
```
It was previously owned by root and links to `/tools/lib`.

You should make the sanity check *before* packaging the package. The supposed out changes from
```
/usr/lib/gcc/i686-pc-linux-gnu/5.2.0/../../../crt1.o succeeded
/usr/lib/gcc/i686-pc-linux-gnu/5.2.0/../../../crti.o succeeded
/usr/lib/gcc/i686-pc-linux-gnu/5.2.0/../../../crtn.o succeeded
```
to
```
/usr/lib/crt1.o succeeded
/usr/lib/crti.o succeeded
/usr/lib/crtn.o succeeded
```
after packaging.

### attr

The package tries to change ownership and permissions of a lot of folders. This can either be skipped by entering 'i' each time (the wrapper scripts in `/usr/lib/pkguser` make this query), or by applying a [patch](../../patches/attr-install-without-chown-and-chmod.patch) which removes that behavior from the install scripts.

### acl

Similar to "attr": the installation tries to change permissions and owners, apply the [patch](../../patches/acl-install-without-chown-and-chmod.patch) to prevent this.

Also make an include directory an install directory:
```BASH
mk-install-dir.sh /usr/include/sys
```
### shadow

It wants to install man pages that we already have (package "man-pages") and translated man pages in `/usr/share/man` - prevent both by applying a [patch](../../patches/shadow-install-without-redundant-manpages.patch).

The package installs the real `su`, now we can remove the wrapper script
```BASH
rm -v /tools/bin/su
```

But we also have to make `su` setuid root (which would be done by the installation, but is prevented by the wrapper scripts):
```BASH
chown root:root /bin/su && \
chmod u+s /bin/su
```

Remove the sticky bit from the other binaries:
```BASH
chmod u-s /usr/bin/{expiry,chage,chsh,gpasswd,newuidmap,newgidmap,newgrp,chfn}
```

Change `/etc/default/useradd` (and make it owned by root):
```BASH
sed -i 's/GROUP=999/GROUP=1000/;s/CREATE_MAIL_SPOOL=yes/CREATE_MAIL_SPOOL=no/' /etc/default/useradd
chown root:root /etc/default/useradd
```

TODO: change the install script to not setup the sticky bit at all.

### e2fsprogs

Skip updating the info file. It needs special handling which is done by a script that requires `sudo` (not yet installed).

### coreutils

Apply the [patch](../../patches/coreutils-8.24-pkg-install.patch).
Configuring with `FORCE_UNSAFE_CONFIGURE=1` is not needed, we aren't root, so it's already safe.

Some existing files are owned by root, but are to be overridden by the package, they need to be changed to be owned by coreutils before installing the package:
```BASH
chown -h coreutils /bin/{echo,cat,stty,pwd}
```

After installing when the files get moved around according to the book, make sure the hash option of the current shell is *off*. Issue `set +h` to be sure.

### m4

A test case fails. This seems to be expected (see LFS mailing list). Continue anyway.

### bash
According to the hint (as root):
```BASH
chown -h bash /bin/bash
```

### libtool

5 tests fail unexpected, but those are fine (circular decencies with "automake").

### perl
Create `/etc/hosts` as root.

As per hint (as root):
```BASH
chown -h perl: /usr/bin/perl
```

After the installation:
```BASH
mk-install-dir.sh /usr/lib/perl5/site_perl/*/
mk-install-dir.sh /usr/lib/perl5/site_perl/*/i686-linux/auto
```

### XML::Parser

I'm lazy and using `cpan`. Some additional module for it: "Bundle::CPAN" is also being installed.

### groff

Before installing delete (as root) the info-dir file (it's like a cache and will be recreated during installation). The file is currently owned by another package.

```BASH
rm -v /usr/share/info/dir
```

### iproute2

Disable building "tipc", which requires libmnl, which is not installed. Without this, the installation fails as it tries to install an non-existing "tipc".
```BASH
sed -i 's/tipc //' Makefile
```

### tar

Test 128 fails. Something with large file support. Couldn't solve it yet, but who needs 8 GB files anyway?

### util-linux

```BASH
mk-install-dir.sh /usr/share/bash-completion/completions
```

### man-db

Like with "attr" and "acl", prevent installation of certain translated man pages by applying the [patch](../../patches/man-db-install-without-redundant-manpages.patch).

### vim

Like above, prevent installation of certain translated man pages by applying the [patch](../../patches/vim-install-without-redundant-manpages.patch).

### uedev

After the installation:
```BASH
mk-install-dir.sh /lib/udev /lib/udev/rules.d
```


TODO: fix hardcodeed path of /bin/mkdir in exec script!
