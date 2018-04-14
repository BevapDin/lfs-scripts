#! /bin/bash

ls -d1rv /packages/*/* | \
    sed '
        /\/Current$/d
        s#/packages/##
    ' | \
    sort -t / -k 1,1 -u
