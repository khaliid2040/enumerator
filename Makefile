# Compiler and flags
CC = gcc
LIBS = 
ifneq ($(wildcard config.mk),)
    include config.mk
endif
# Directories
SRC_DIR_EXTRA = extra
SRC_DIR_NET = net
OBJ_DIR = obj
BIN_DIR = ~/.local/bin

# Executable name
EXECUTABLE = systeminfo

# List of object files
OBJ_FILES = $(OBJ_DIR)/main.o $(OBJ_DIR)/extra_func.o $(OBJ_DIR)/storage.o \
            $(OBJ_DIR)/memory.o $(OBJ_DIR)/cpuinfo.o $(OBJ_DIR)/process.o \
            $(OBJ_DIR)/network.o $(OBJ_DIR)/route.o $(OBJ_DIR)/arp.o \
            $(OBJ_DIR)/system.o $(OBJ_DIR)/security.o $(OBJ_DIR)/packages.o

# Main target
$(EXECUTABLE): $(OBJ_FILES)
	$(CC) -o $@ $(OBJ_FILES) $(LDFLAGS) $(LIBS)

# Ensure object directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile object files from the main directory
$(OBJ_DIR)/main.o: main.c main.h | $(OBJ_DIR)
	$(CC) -c -o $@ main.c $(CFLAGS)

# Compile object files from the extra directory
$(OBJ_DIR)/%.o: $(SRC_DIR_EXTRA)/%.c main.h | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Compile object files from the net directory
$(OBJ_DIR)/%.o: $(SRC_DIR_NET)/%.c main.h | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

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

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE) config.mk
