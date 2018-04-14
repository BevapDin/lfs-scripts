#!/bin/bash
modprobe -l | grep -v -E "^($(lsmod | cut -d" " -f1 | tail -n +2 | xargs modinfo -n | tr "\n" "|"))$"

