#! /bin/bash

remote="$1"
branch="$2"

git checkout "$remote/$branch" || exit $?
git checkout -b "$branch" || exit $?
git push -d "$remote" "$branch" || exit $?

exit 0
