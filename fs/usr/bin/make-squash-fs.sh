#! /bin/bash

exec mksquashfs "$@" -comp lz4 -no-exports -no-xattrs -all-root -Xhc
