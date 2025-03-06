# Include config.mk to load CFLAGS, LDFLAGS, and other settings
ifneq ($(wildcard config.mk),)
    include config.mk
endif
# Directories for source and object files
SRC_DIR_EXTRA = extra
SRC_DIR_NET = net
SRC_DIR_SYSTEM = system
OBJ_DIR_EXTRA = extra
OBJ_DIR_NET = net
OBJ_DIR_SYSTEM = system

BIN_DIR = ~/.local/bin

# Executable name
EXECUTABLE = systeminfo

# Object files
OBJ_FILES = main.o \
			$(OBJ_DIR_EXTRA)/process.o $(OBJ_DIR_EXTRA)/utils.o $(OBJ_DIR_EXTRA)/desktop.o \
			$(OBJ_DIR_EXTRA)/security.o $(OBJ_DIR_EXTRA)/packages.o $(OBJ_DIR_EXTRA)/shell.o \
			$(OBJ_DIR_NET)/network.o $(OBJ_DIR_NET)/arp.o $(OBJ_DIR_NET)/route.o \
			$(OBJ_DIR_SYSTEM)/cpuinfo.o $(OBJ_DIR_SYSTEM)/memory.o $(OBJ_DIR_SYSTEM)/storage.o $(OBJ_DIR_SYSTEM)/display.o \
			$(OBJ_DIR_SYSTEM)/pci.o $(OBJ_DIR_SYSTEM)/power.o $(OBJ_DIR_SYSTEM)/system.o $(OBJ_DIR_SYSTEM)/virt.o

# Verbosity flag for cleaner output
V ?= 0

# Main target to build the executable
$(EXECUTABLE): $(OBJ_FILES)
	@if [ $(V) -eq 1 ]; then echo "CC -o $@ $(OBJ_FILES) $(LDFLAGS)"; else echo "LD   $@"; fi
	@$(CC) -o $@ $(OBJ_FILES) $(LDFLAGS)

# Compile object files from the extra directory
$(OBJ_DIR_EXTRA)/%.o: $(SRC_DIR_EXTRA)/%.c main.h | $(OBJ_DIR_EXTRA)
	@if [ $(V) -eq 1 ]; then echo "CC -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ $< $(CFLAGS)

# Compile object files from the net directory
$(OBJ_DIR_NET)/%.o: $(SRC_DIR_NET)/%.c main.h | $(OBJ_DIR_NET)
	@if [ $(V) -eq 1 ]; then echo "CC -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ $< $(CFLAGS)

# Compile object files from the system directory
$(OBJ_DIR_SYSTEM)/%.o: $(SRC_DIR_SYSTEM)/%.c main.h | $(OBJ_DIR_SYSTEM)
	@if [ $(V) -eq 1 ]; then echo "CC -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o $@ $< $(CFLAGS)

# Compile main.o in the same way as others
main.o: main.c main.h
	@if [ $(V) -eq 1 ]; then echo "CC -c -o $@ $< $(CFLAGS)"; else echo "CC   $@"; fi
	@$(CC) -c -o main.o main.c $(CFLAGS)

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

# Help message
help:
	@echo "make checkdep"
	@echo "make"
	@echo "make install (optional)"

# Clean up build files
clean:
	rm -rf $(OBJ_DIR_EXTRA)/*.o
	rm -rf $(OBJ_DIR_NET)/*.o
	rm -rf $(OBJ_DIR_SYSTEM)/*.o
	rm -f *.o config.mk $(EXECUTABLE)
