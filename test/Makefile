# Makefile for cross-compiling a project

# Toolchain prefix, can be set from the command line or environment
CROSS_COMPILE ?= arm-linux-gnueabihf-

# Compiler and archiver
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# Source files and directories
SRCDIR = .
OBJDIR = build
INCDIR = .

# Installation directory
PREFIX = /usr/local
DESTDIR ?= $(PWD)
INSTALL_DIR = $(DESTDIR)/_install

# Source files (relative to SRCDIR)
SRC = \
    $(SRCDIR)/pdm_test.c \
    $(SRCDIR)/pdm_dimmer_test.c \
    $(SRCDIR)/pdm_nvmem_test.c \
    $(SRCDIR)/pdm_sensor_test.c \
    $(SRCDIR)/pdm_switch_test.c

# Object files (relative to OBJDIR)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

# Target executable name
TARGET = pdm_test

# CFLAGS: compiler flags
CFLAGS += -Wall -Wextra -I$(INCDIR)

# header files
CFLAGS += -I../include/uapi

# LDFLAGS: linker flags
LDFLAGS =

# Default target
all: clean $(TARGET)

# Rule to create object directory
$(OBJDIR):
	@mkdir -p $@

# Rule to compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

# Link object files into the final executable
$(TARGET): $(OBJS)
	@$(CC) $(LDFLAGS) -o $@ $^

# Install the target executable
install:
	@mkdir -p $(INSTALL_DIR)/bin
	@cp $(TARGET) $(INSTALL_DIR)/bin/

# Uninstall the target executable
uninstall:
	@rm -f $(INSTALL_DIR)/bin/$(TARGET)

# Clean up generated files
clean:
	@rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean install uninstall
