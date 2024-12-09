export SHELL := /bin/bash

AS = nasm
AFLAGS_16 = -f bin
AFLAGS_32 = -f elf -g -F stabs

CC = gcc
CFLAGS = -m32 -c -ffreestanding -mgeneral-regs-only

LD = ld
LDARGS = -T link.ld -melf_i386

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


CC_SOURCES := $(wildcard $(SOURCEDIR)/*.c) $(wildcard $(SOURCEDIR)/*/*.c)
AS_SOURCES := $(wildcard $(SOURCEDIR)/*.s) $(wildcard $(SOURCEDIR)/*/*.s)

HEADERS := $(wildcard $(SOURCEDIR)/*.h)


OBJECTS := $(patsubst $(SOURCEDIR)/%.c, $(BUILDDIR)/%.o, $(CC_SOURCES))
OBJECTS += $(patsubst $(SOURCEDIR)/%.s, $(BUILDDIR)/%.o, $(AS_SOURCES))

.PHONY: copy-files

all: $(TARGET)

$(TARGET): $(KERNEL) $(BOOTLOADER) copy-files
	cat $(BOOTLOADER) $(KERNEL) >> $@
	truncate -s 66048 $@
	cat $(FILESYSTEM) >> $@

# Building bootloader sector
$(BOOTLOADER): $(BOOTSOURCE)
	$(AS) $(AFLAGS_16) $< -o $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(AFLAGS_32) $< -o $@
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I $(SOURCEDIR) $< -o $@

$(KERNEL): $(OBJECTS)
	@echo $(OBJECTS)
	$(LD) $(LDARGS) -o $(BUILDDIR)/kernel.elf $(OBJECTS)
	objcopy -O binary $(BUILDDIR)/kernel.elf $@


copy-files: $(FILESYSTEM)
	@echo copying files...
	cp -r $(FSINITDIR)/* $(FSMOUNTDIR)
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
	@-sudo umount fs/mountpoint
	@-sudo losetup -d /dev/loop0
	@-rm fs/filesystem.img

clean: fs-clean
	@-rm -r $(BUILDDIR)/*