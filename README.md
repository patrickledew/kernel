# Homebrew bootloader and kernel

This projects is intended to help me understand how bootloaders and kernels work on the most fundamental level. I will be using it to explore different OS concepts and their implementation, such as system calls, file systems, IPC and more.

## Guide to bin/ folder

### `main.img`

The most important file in this folder is `main.img`, as it is the flat binary image that gets loaded into QEMU as the boot disk. The first 512 bytes of this are the bootloader, and the rest are the kernel, which has been compiled and linked into it's own flat binary, `kernel.bin`.

### `kernel.bin`

This is the flat binary that contains the entire kernel. It is generated by first assembling/compiling the source files, then linking them together via the linker script `link.ld`. This generates a `kernel.elf` file, which is then run through `objcopy` to remove all symbols/ELF headers and leave the raw binary.

### `kernel.elf`

This is the ELF-formatted binary version of the kernel, the result of linking all the compiled object files together. This contains debugging information which is helpful when using gdb. The `debug.sh` script makes use of this file.

### `boot.bin`

This is a flat binary containing the bootloader. Since this runs in real mode (16-bit), it can not be linked using `ld`.

### `*.o`

The rest of the files are .o files, and are ELF format as well. There is one for each source file (apart from boot.s) and these are all linked together to create `kernel.elf`.
