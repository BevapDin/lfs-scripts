# Chapter 5 of the book

## Package manager

Read the LFS hint [More Control and Package Management using Package Users](http://www.linuxfromscratch.org/hints/downloads/files/more_control_and_pkg_man.txt) and download its [attachment](http://www.linuxfromscratch.org/hints/downloads/files/ATTACHMENTS/more_control_and_pkg_man/more_control_helpers.tar.bz2). The archive goes into `$LFS/tools` and is to be extracted there:

```BASH
cd $LFS/tools && \
wget http://www.linuxfromscratch.org/hints/downloads/files/ATTACHMENTS/more_control_and_pkg_man/more_control_helpers.tar.bz2 && \
tar -xf more_control_helpers.tar.bz2
```

## Install chapter 5 as per book.

## coreutils:

Apply the [patch](../../patches/coreutils-8.24-pkg-install.patch). It changes the behavior of the `install` program a bit to prevent stuff that is usually not possible as install user anyway.

The hint says to install `su` from here, but "coreutils" no longer provides it. The one from "util-linux" requires PAM, which we don't have at that point. Therefor install a [wrapper script](../../usr/sbin/su).

## Final step to make the package manager usable

At the end of Chapter 5 (before changing ownership!) install the scripts `groupadd` and `useradd` from the hint:
```BASH
cp -v $LFS/tools/more_control_helpers/sbin/{groupadd,useradd} $LFS/tools/bin
```

Install the scripts [`install_package`](../../usr/sbin/install_package) and [`add_package_user`](../../usr/sbin/add_package_user) from the current LFS system, I like them more (-;

Note: if you intend to use the new system as a direct replacement for an existing system, you may want to set up user accounts for the users of the existing system. At this point, there are only the standard user accounts defined, which should exist in the other system as well, but that will change as soon as you add new user accounts for the install users.

Now you can add user accounts whose *ids* match the ids of the same account on the existing system. This allows you to use the files from that other system directly without changing their ownership.

Continue with [chapter 6](chapter6.md).
