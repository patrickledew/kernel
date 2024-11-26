rm -r bin/*

# Generate object files
nasm -f bin src/boot.s -o bin/boot.bin -l boot.lst # For boot sector, since this is 16 bit, cannot use ELF
nasm -f bin src/kernel_init.s -o bin/kernel_init.bin -l kernel.lst

# Link the elf files to correct memory addresses
# ld -mi386pe bin/kernel_init.o -o bin/kernel_init.elf --section-start=kernel=0x7e00

# Objcopy any elf files we want in complete image to flat binaries
# objcopy -I pe-i386 -O binary bin/kernel_init.o bin/kernel_init.bin

# Concat all the bin files into one image
cat bin/boot.bin bin/kernel_init.bin >> bin/main.img

qemu-system-i386 -hda bin/main.img -S -s -d int &
gdb -ex 'target remote localhost:1234' \
    -ex 'set architecture i8086' \
    -ex 'break *0x7c00' \
    -ex 'continue'