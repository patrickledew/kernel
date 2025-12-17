KLIB = ../klib
KLIB_LIB = $(KLIB)/bin/libklib.a
KLIB_INC = $(KLIB)/include

CC = gcc
CFLAGS = -m32 -c -ffreestanding -mgeneral-regs-only -g -ggdb -I$(KLIB_INC)

LD = ld
LDARGS = -melf_i386 -e main

# Find all subdirectories
SUBDIRS := $(wildcard src/*/)

# Default target
all: $(SUBDIRS)

# Pattern rule for subdirectories
$(SUBDIRS):
	$(eval SOURCES := $(wildcard $@*.c))
	@echo $(SOURCES)
	$(eval OBJECTS := $(SOURCES:.c=.o))
	@mkdir -p bin
	@echo "Building $@..."
	for src in $(SOURCES) ; do \
		$(CC) $(CFLAGS) -o $$(echo $$src | sed 's/\.c/\.o/') $$src ; \
	done
	$(LD) $(LDARGS) -o bin/$(notdir $(@:%/=%)).elf $(OBJECTS) -L$(dir $(KLIB_LIB)) -lklib

clean:
	rm -f */*.o bin/*.elf
	rmdir bin 2>/dev/null || true

.PHONY: all clean $(SUBDIRS)
