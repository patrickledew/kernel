#!/bin/sh
cd fs
sudo losetup /dev/loop0 filesystem.img
sudo mount -o uid=1000 -t msdos /dev/loop0 mountpoint

