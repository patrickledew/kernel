#!/usr/bin/env sh

make clean
make || exit


# add -d int if you want to see the interrupts
qemu-system-i386 -hda bin/main.img -s -S -M smm=off &
# gdb bin/kernel.elf -ex 'target remote localhost:1234' \
#     -ex 'set architecture i386' \1
#     -ex 'break kmain' 
    # -ex 'continue'

ugdb bin/kernel.elf -x debug-commandfile.txt