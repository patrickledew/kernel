CC = gcc
CFLAGS = -m32 -c -ffreestanding -mgeneral-regs-only -g -ggdb

LD = ld
LDARGS = -melf_i386 -e main

TARGET := bin/hello.elf
OBJ := hello.o
SOURCE := hello.c

all: $(TARGET)
	-rm $(OBJ)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $<
	$(LD) $(LDARGS) -o $@ $(OBJ)