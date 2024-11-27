AS = nasm
AFLAGS_16 = -f bin
AFLAGS_32 = -f elf -g -F stabs

CC = gcc
CFLAGS = -m32 -c -ffreestanding

LD = ld
LDARGS = -T link.ld -melf_i386

BOOTSOURCE = src/boot/boot.s # Only file needed for bootloader

SOURCEDIR = src/kernel

BUILDDIR = bin

BOOTLOADER = $(BUILDDIR)/boot.bin
KERNEL = $(BUILDDIR)/kernel.bin

TARGET = $(BUILDDIR)/main.img


CC_SOURCES := $(wildcard $(SOURCEDIR)/*.c)
AS_SOURCES := $(wildcard $(SOURCEDIR)/*.s)


OBJECTS := $(patsubst $(SOURCEDIR)/%.c, $(BUILDDIR)/%.o, $(CC_SOURCES))
OBJECTS += $(patsubst $(SOURCEDIR)/%.s, $(BUILDDIR)/%.o, $(AS_SOURCES))

all: $(TARGET)

$(TARGET): $(KERNEL) $(BOOTLOADER)
	cat $(BOOTLOADER) $(KERNEL) >> $@

# Building bootloader sector
$(BOOTLOADER): $(BOOTSOURCE)
	$(AS) $(AFLAGS_16) $< -o $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.s
	$(AS) $(AFLAGS_32) $< -o $@
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

$(KERNEL): $(OBJECTS)
	$(LD) $(LDARGS) -o $(BUILDDIR)/kernel.elf $(OBJECTS)
	objcopy -O binary $(BUILDDIR)/kernel.elf $@


clean:
	rm -r $(BUILDDIR)/*