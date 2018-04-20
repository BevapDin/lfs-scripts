#! /bin/bash

touch obj/*.o
sleep 1
git diff --name-only ^@{1}|grep '\.cpp$'|xargs touch
