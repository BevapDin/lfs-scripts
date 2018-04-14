#! /bin/bash

grep -le '^'"$1"'$' /usr/src/*/.needs-pkg | \
	sed 's#^/usr/src/##;s#/.*$##'
