#! /bin/bash

diff -y .config $HOME/.config.allmods | \
	grep -E '^(.*)=y.*\1=m'
