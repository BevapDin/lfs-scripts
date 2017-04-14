#! /bin/bash

cd /usr/src || exit $?

for d in * ; do
	echo "prog '$d' - xterm -e inst-bash '$d'"
done
