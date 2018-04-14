#! /bin/bash

git log --reverse --pretty=format:%H --first-parent --merges ..upstream/master | head -1
