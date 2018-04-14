#! /bin/bash

echo "Making build dir and cd to it ..."
mkdir build || exit $?
cd build || exit $?
echo "... done."

echo "cmake ..."
cmake \
	-DCMAKE_INSTALL_PREFIX=$KDE4_PREFIX \
	-DKDE_DEFAULT_HOME=.kde4 \
	-DSYSCONF_INSTALL_DIR=/etc/kde4 \
	.. || exit $?
echo "... done."

echo "ccmake ..."
ccmake . || exit $?
echo "... done."

if ! make ; then
	echo "#################"
	echo "## Make failed ##"
	echo "#################"
	ret=$?
	exit $ret
fi

if ! make install ; then
	echo "#########################"
	echo "## Make install failed ##"
	echo "#########################"
	ret=$?
	exit $ret
fi

exit 0

