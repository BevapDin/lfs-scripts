# Installing X

```
cat > /etc/profile.d/xorg_conf.sh << "EOF"
XORG_PREFIX="/usr"
XORG_CONFIG="--prefix=$XORG_PREFIX --sysconfdir=/etc --localstatedir=/var --disable-static"
export XORG_PREFIX XORG_CONFIG
EOF
chmod 644 /etc/profile.d/xorg_conf.sh
```

Copy the most useful helper [script](../../usr/sbin/install-xorg-packages.sh). It takes only a single URL as argument and does:
- determine the install user name,
- download and unpack the package,
- runs `configure`, `make` and `make install`,
- pack the installation (move installed files to `/packages` using `tpkgs`).

It also requires a folder `/s/to-canopus` to exists and to be writable by install users, make it so:
```BASH
mkdir -v -p /s/to-canopus
chown -v :install /s/to-canopus
chmod -v g+w,o+t /s/to-canopus
```
`mk-install-dir.sh` could be used, but we don't want this directory to appear in the list of install directories `/etc/install-dirs`. If you use the script (which adds the directory to the list), remove it from the list afterwards.

A few folders need to be made isntall directories:
```BASH
mk-install-dir.sh /usr/include/X11 /usr/include/X11/{extensions,fonts}
```

### xcb-proto

The package installs python modules. This requires the python module directory to be an install directory:
```BASH
mk-install-dir.sh /usr/lib/python3.4/site-packages
```
Note: the path very much depends on what python version the package uses. However, it's acceptable (and maybe even a good thing) to make those "site-packages" folders into install directories for every installed python version.
```BASH
mk-install-dir.sh /usr/lib/python*/site-packages
```

### fonts

Make some install directories:
```BASH
mk-install-dir.sh /usr/share/fonts/X11/encodings/large
mk-install-dir.sh /usr/share/fonts/X11/{100dpi,75dpi,Type1,cyrillic,misc,TTF,OTF}
mk-install-dir.sh /etc/fonts/conf.avail
mk-install-dir.sh /etc/fonts/conf.d
```

Some installations can fail because they call `mkfontscale`, `mkfontdir` and `fc-cache`, which write cache files into the folders contained in `/usr/share/fonts/X11`. The first package to do this will succeed, but all the other packages try to override existing files, which fails.

To fix this: remove the cache files after the installation (but before the packaging) as root:
```BASH
rm -v /usr/share/fonts/X11/*/{fonts.dir,fonts.scale}
```

If they are not removed before packaging, they'll end up in `/packages`, where they don't belong. Use the command above to remove the installed symlinks, and use this command to remove them from /packages:
```BASH
rm -v /packages/font*/Current/usr/share/fonts/X11/*/{fonts.dir,fonts.scale}
```

Finally, when all packages have been installed, run this:
```BASH
for d in 100dpi 75dpi cyrillic misc OTF TTF Type1 ; do
    mkfontscale "/usr/share/fonts/X11/$d"
    mkfontdir "/usr/share/fonts/X11/$d"
    fc-cache "/usr/share/fonts/X11/$d"
done
```

### xkeyboard-config

```BASH
mk-install-dir.sh /usr/src/xkeyboard-config/pkg/answer
```

### xorg-server

Add `--disable-install-setuid` to configure so it won't have to test whether `chmod` and `chown` work.
Add `--libexec=/usr/lib/`, which for some reason is not done by `cfgc`.

```BASH
mk-install-dir.sh /usr/share/X11/xkb/compiled
```

Enable the setuid wrapper:
```BASH
chown -v root `readlink /usr/lib/Xorg.wrap`
chmod -v u+s `readlink /usr/lib/Xorg.wrap`
```

### xf86-input-evdev

```BASH
mk-install-dir.sh /usr/lib/xorg/modules
mk-install-dir.sh /usr/include/xorg
```

### xf86-video-ati

Needs to be configured with `--disable-glamor`.

```BASH
mk-install-dir.sh /usr/lib/xorg/modules/drivers
```

### xterm

```BASH
mk-install-dir.sh /etc/X11/app-defaults
```

The command `make install-ti` is not run, we'll see how that works out.


