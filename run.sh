#!/usr/bin/env sh

make clean
make || exit

qemu-system-i386.exe -drive format=raw,file=bin/main.img