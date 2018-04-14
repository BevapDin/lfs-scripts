#! /bin/sh

INSTALL_DIR_LIST=/etc/install-dirs
INSTALL_OWNER=root:root
INSTALL_PERM=0755

if ! [ -d "$1" ] ; then
	exit 0
fi
ls -ldh --color=auto "$1"

sed 's#^'"$1"'/*$##; /^$/ d' -i "$INSTALL_DIR_LIST"

chown $INSTALL_OWNER "$1"
chmod $INSTALL_PERM "$1"

