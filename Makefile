# Compiler and flags
CC = gcc
LIBS = 
ifneq ($(wildcard config.mk),)
    include config.mk
endif
# Directories
SRC_DIR_EXTRA = extra
SRC_DIR_NET = net
SRC_DIR_SYSTEM = system
OBJ_DIR = obj
BIN_DIR = ~/.local/bin

# Executable name
EXECUTABLE = systeminfo

# List of object files
OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/storage.o \
            $(OBJ_DIR)/memory.o $(OBJ_DIR)/cpuinfo.o $(OBJ_DIR)/process.o \
            $(OBJ_DIR)/network.o $(OBJ_DIR)/route.o $(OBJ_DIR)/arp.o \
            $(OBJ_DIR)/system.o $(OBJ_DIR)/security.o $(OBJ_DIR)/packages.o \
			$(OBJ_DIR)/virt.o $(OBJ_DIR)/power.o $(OBJ_DIR)/pci.o

# Toggle verbosity (default is 0 for cleaner output)
V ?= 0

# Main target
$(EXECUTABLE): $(OBJ_FILES)
	@if [ $(V) -eq 1 ]; then echo "gcc -o $@ $(OBJ_FILES) $(LDFLAGS) $(LIBS)"; else echo "LD   $@"; fi
	@$(CC) -o $@ $(OBJ_FILES) $(LDFLAGS) $(LIBS)

# Ensure object directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile object files from the root directory
$(OBJ_DIR)/main.o: main.c main.h | $(OBJ_DIR)
	@if [ $(V) -eq 1 ]; then echo "gcc -c -o $@ main.c $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ main.c $(CFLAGS)

# Compile object files from the extra directory
$(OBJ_DIR)/%.o: $(SRC_DIR_EXTRA)/%.c main.h | $(OBJ_DIR)
	@if [ $(V) -eq 1 ]; then echo "gcc -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ $< $(CFLAGS)

# Compile object files from the system directory
$(OBJ_DIR)/%.o: $(SRC_DIR_SYSTEM)/%.c main.h | $(OBJ_DIR)
	@if [ $(V) -eq 1 ]; then echo "gcc -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ $< $(CFLAGS)

# Compile object files from the net directory
$(OBJ_DIR)/%.o: $(SRC_DIR_NET)/%.c main.h | $(OBJ_DIR)
	@if [ $(V) -eq 1 ]; then echo "gcc -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ $< $(CFLAGS)

# Phony targets
.PHONY: all install checkdep clean

# Default target
all: $(EXECUTABLE)

# Install target
install: $(EXECUTABLE)
	@echo "Checking .local"
	@if [ -d $(BIN_DIR) ]; then \
		mv -v $(EXECUTABLE) $(BIN_DIR); \
	else \
		mkdir -pv $(BIN_DIR); \
		mv -v $(EXECUTABLE) $(BIN_DIR); \
	fi

# Check dependencies
checkdep:
	@./config.sh

# for just simple help instead of person reading entire readme
help:
	@echo "make checkdep"
	@echo "make"
	@echo "make install (	optional)"
# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE) config.mk
