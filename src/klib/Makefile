# Compiler and flags
CC = gcc
CFLAGS = -ffreestanding -Iinclude -m32
AR = ar
ARFLAGS = rcs

# Directories
BIN_DIR = bin
OBJ_DIR = obj

# Source and object files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

# Target library
TARGET = $(BIN_DIR)/libklib.a

# Make sure the directories exist
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR))

# Default target
all: $(TARGET)

# Create the static library
$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# Compile source files
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

.PHONY: all clean