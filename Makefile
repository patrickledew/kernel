export SHELL := /bin/bash

AS = nasm
AFLAGS_16 = -f bin
AFLAGS_32 = -f elf32 -F dwarf -g

CC = gcc
CFLAGS = -m32 -c -ffreestanding -mgeneral-regs-only -g -ggdb

LD = ld
LDARGS = -T link.ld -melf_i386 -g --whole-archive

BOOTSOURCE = src/boot/boot.s # Only file needed for bootloader

SOURCEDIR = src/kernel

BUILDDIR = bin

FSDIR = fs

BOOTLOADER = $(BUILDDIR)/boot.bin
KERNEL = $(BUILDDIR)/kernel.bin
FILESYSTEM = $(FSDIR)/filesystem.img
FSMOUNTDIR = $(FSDIR)/mountpoint
FSINITDIR = $(FSDIR)/init

TARGET = $(BUILDDIR)/main.img


CC_SOURCES := $(shell find $(SOURCEDIR) -name '*.c')
AS_SOURCES := $(shell find $(SOURCEDIR) -name '*.s')

OBJECTS := $(patsubst $(SOURCEDIR)/%.c, $(BUILDDIR)/%.o, $(CC_SOURCES))
OBJECTS += $(patsubst $(SOURCEDIR)/%.s, $(BUILDDIR)/%.o, $(AS_SOURCES))

HOSTED_DIR := src/programs
HOSTED_BIN := $(HOSTED_DIR)/bin

.PHONY: copy-files

all: $(TARGET)


# Reserved space for boot sector/kernel = 0x20200
# everything after that is reserved for the filesystem
$(TARGET): $(KERNEL) $(BOOTLOADER) copy-files
	cat $(BOOTLOADER) $(KERNEL) >> $@
	truncate -s 131584 $@
	cat $(FILESYSTEM) >> $@

# Building bootloader sector
$(BOOTLOADER): $(BOOTSOURCE)
	$(AS) $(AFLAGS_16) $< -o $@ -l $<.lst

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(AFLAGS_32) $< -o $@
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I $(SOURCEDIR) $< -o $@

$(KERNEL): $(OBJECTS)
	@echo $(CC_SOURCES)
	@echo $(AS_SOURCES)
	@echo $(OBJECTS)
	$(LD) $(LDARGS) -o $(BUILDDIR)/kernel.elf $(OBJECTS)
	objcopy -O binary $(BUILDDIR)/kernel.elf $@


copy-files: $(FILESYSTEM) programs
	@echo copying files...
	cp -r $(FSINITDIR)/* $(FSMOUNTDIR)
	cp -r $(HOSTED_BIN)/* $(FSMOUNTDIR)
	sudo sync -f $(FSMOUNTDIR)/*
	ls $(FSMOUNTDIR)

.ONESHELL:
$(FILESYSTEM):
	sudo mkfs.fat -F 12 -C $@ 1024
	sudo losetup /dev/loop0 $@
	sudo mount -o uid=1000 -t msdos /dev/loop0 $(FSMOUNTDIR)

	@echo Created filesystem image in $(FILESYSTEM)



fs: fs-clean $(FILESYSTEM)

fs-clean:
	@-sudo umount -l fs/mountpoint
	@-sudo umount -l /mnt/c/users/ricky/development/c_c++/kernel/fs/mountpoint
	@-sudo umount -l /mnt/c/Users/Ricky/Development/C_C++/kernel/fs/mountpoint
	@-sudo losetup -d /dev/loop0
	@-rm fs/filesystem.img

clean: fs-clean
	@-rm -r $(BUILDDIR)/*

klib: 
	@make clean -C src/klib
	@make -C src/klib

programs: klib
	@make clean -C $(HOSTED_DIR)
	@make -C $(HOSTED_DIR)