#! /bin/bash

swapdev=/dev/hdb9
swap=/dev/mapper/swap

swapoff "$swap" && \
cryptsetup luksClose swap && \
mkswap "$swapdev" && \
swapon "$swapdev" && \
true && \
true && \
true

