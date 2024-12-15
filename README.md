# Homebrew bootloader and kernel

This projects is intended to help me understand how bootloaders and kernels work on the most fundamental level. I will be using it to explore different OS concepts and their implementation, such as system calls, file systems, IPC and more.

## Build Process
The entire build process is controlled through the Makefile in the root of this repo. If you're on windows, it's required to use WSL in order to build (because of the way the filesystem is created we need `mount`, `mkfs`, etc). Explained shortly, the main build steps are:

1. Build the bootloader (boot.s, gdt.s). This is all in assembly, and also run in 16-bit real mode, so `nasm` is used to assemble it into a raw binary.
2. Assemble the kernel initialization code (kernel_init.s). This is the first code run in 32-bit protected mode, and so can be linked with `ld` unlike the 16-bit bootloader code. It is assembled to an object file and will be linked along with the rest of the kernel.
3. Compile each .c file into a corresponding object file.
4. Link object files together into kernel.elf using the linker script `link.ld`. This arranges the different sections (especially .kernel_init) in the correct order so the bootloader can jump to our code. It also defines where we're loading our kernel into memory.(0x10000-0x1FFFF)
5. kernel.elf is still in ELF format (containing symbols, debug information, etc.) which we cannot directly run without additional bootstrapping code. In order to simplify things, this is reduced down to a flat binary using objcopy to produce kernel.bin.
6. Next, we build the filesystem. First the empty FAT12 filesystem is created with `mkfs.fat`, and we create a loopback device and mount it at `fs/mountpoint/` (unfortunately this requires sudo privileges, which is annoying but necessary until I find a better solution), and finally files from `fs/init/` are copied in. After a `sync` to ensure everything is written to the filesystem, we have the final filesystem image `fs/filesystem.img`.
7. Now we finally bring everything together by `cat`-ing everything into a single file. First, the bootloader binary and kernel binary are combined into `main.img`. Since we want the filesystem to start at 0x10200 (and not just wherever the end of the kernel image is), this is padded to be exactly 0x10200 bytes. Finally `filesystem.img` is appended and we have the full disk image, ready to be run by QEMU or copied to physical media. 

## Scripts

`run.sh`: Compiles and runs the kernel using QEMU.
`debug.sh`: Compiles kernel and runs QEMU with debugging enabled, then attaches a gdb instance.