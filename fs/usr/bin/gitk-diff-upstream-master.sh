#! /bin/bash

branch="$(git branch|grep -E '^\* '|sed 's#\* ##')"

exec gitk "upstream/master...$branch" &
