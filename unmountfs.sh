#!/bin/sh
cd fs
sudo umount mountpoint
sudo losetup -d /dev/loop0
