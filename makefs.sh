#!/bin/sh
cd fs
rm filesystem.img
sudo mkfs.fat -F 12 -C filesystem.img 50 # 50 * 1024 bytes = 50kiB
cd ..
sh mountfs.sh