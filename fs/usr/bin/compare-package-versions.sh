#! /bin/bash

PKG="$1"
V1="$2"
V2="$3"

shift 3
D1="/packages/$PKG/$V1"
D2="/packages/$PKG/$V2"
SEDS='s#^/packages/[^/]*/[^/]*/##'
#SEDS='s#^/packages/[^/]*/[^/]*/##;s#^usr/##'

diff "$@" <(find "$D1" | sed "$SEDS") <(find "$D2" | sed "$SEDS")

