#!/bin/sh
cd fs
rm filesystem.img
sudo mkfs.fat -F 12 -C filesystem.img 1024 # 1024 * 1024 bytes = 1MiB
cd ..
sh mountfs.sh