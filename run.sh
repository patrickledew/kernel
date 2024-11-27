#!/usr/bin/env sh

make clean
make || exit

qemu-system-i386.exe bin/main.img