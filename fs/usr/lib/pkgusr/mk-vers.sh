#! /bin/bash

version="${1?Missing version}"

mkdir -p "/packages/$USER" || exit $?

tpkgs --pack "$USER" "$version" || exit $?

exit 0
