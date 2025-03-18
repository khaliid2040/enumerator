#!/bin/bash

LDFLAGS=""
CFLAGS="-march=native -pipe -I."
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[33m'
NC='\033[0m'

# Define theme colors
declare -A THEMES=(
    ["red"]="-DRED"
    ["green"]="-DGREEN"
    ["yellow"]="-DYELLOW"
    ["magenta"]="-DMAGENTA"
)

# Usage function
usage() {
    echo "Usage: $0 [-c <color>] [-d] [-s]"
    exit 1
}

# Parse command-line arguments
while getopts "c:ds" opt; do
    case "$opt" in
        c)
            if [[ -v THEMES[${OPTARG}] ]]; then
                CFLAGS+=" ${THEMES[${OPTARG}]}"
                echo -e "Theme ${OPTARG}: ${GREEN}OK${NC}"
            else
                CFLAGS+=" -DDEFAULT"
                echo -e "Theme ${OPTARG}: ${RED}Unsupported${NC}"
                echo -e "${YELLOW}Warning: color defaulting to green light${NC}"
            fi
            ;;
        d)
            echo -e "Debugging: ${GREEN}ON${NC}"
            CFLAGS+=" -Og -g -DDEBUG"
            ;;
        s)
            CFLAGS+=" -fsanitize=address -fsanitize=leak -fsanitize=undefined -fanalyzer -Wno-analyzer-use-of-uninitialized-value"
            LDFLAGS+=" -fsanitize=address -fsanitize=leak -fsanitize=undefined"
            echo -e "Sanitizer: ${GREEN}ON${NC}"
            ;;
        *)
            usage
            ;;
    esac
done

# Default optimizations if debugging is off
if [[ ! $CFLAGS =~ "-DDEBUG" ]]; then
    CFLAGS+=" -O2"
    LDFLAGS+=" -flto"
fi

# Libraries to check
LIBS=("libudev" "libsystemd" "libwayland")
LIBPCI="libpci"

# Check for the standard libraries
for LIB in "${LIBS[@]}"; do
    case $LIB in
        "libudev")
            if [ -f /usr/include/libudev.h ]; then
                CFLAGS+=" -DLIBUDEV"
                LDFLAGS+=" -ludev"
                echo -e "Checking ${LIB}: ${GREEN}OK${NC}"
            else
                echo -e "Checking ${LIB}: ${RED}NO${NC}"
            fi
            ;;
        "libsystemd")
            if [ -d /usr/include/systemd ]; then
                CFLAGS+=" -DSYSTEMD"
                LDFLAGS+=" -lsystemd"
                echo -e "Checking ${LIB}: ${GREEN}OK${NC}"
            else
                echo -e "Checking ${LIB}: ${RED}NO${NC}"
            fi
            ;;
        "libwayland")
            if [ -f /usr/include/wayland-client.h ]; then
                CFLAGS+=" -DLIBWAYLAND"
                LDFLAGS+=" -lwayland-client"
                echo -e "Checking ${LIB}: ${GREEN}OK${NC}"
            else
                echo -e "Checking ${LIB}: ${RED}NO${NC}"
            fi
            ;;
    esac
done

# Detect security modules
if [ -d "/sys/fs/selinux" ]; then
    CFLAGS+=" -DSELINUX_H"
fi
if [ -d "/sys/kernel/security/apparmor" ]; then
    CFLAGS+=" -DAPPARMOR_H"
fi

# Detect distribution and check for libpci
detect_distribution() {
    local path="$1"
    if [ -d "${path}/pci" ]; then
        CFLAGS+=" -DLIBPCI"
        LDFLAGS+=" -lpci"
        echo -e "Checking libpci: ${GREEN}OK${NC}"
    else
        echo -e "Checking libpci: ${RED}NO${NC}"
    fi
}

if [ -f /usr/bin/apt ]; then
    CFLAGS+=" -DDEBIAN"
    detect_distribution "/usr/include/x86_64-linux-gnu"
    echo -e "Detected Debian-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/rpm ]; then
    detect_distribution "/usr/include"
    if [ -f /usr/bin/dnf ]; then
        CFLAGS+=" -DREDHAT"
        echo -e "Detected Redhat-based distribution: ${GREEN}OK${NC}"
    elif [ -f /usr/bin/zypper ]; then
        CFLAGS+=" -DOPENSUSE"
        echo -e "Detected Opensuse distribution: ${GREEN}OK${NC}"
    fi   
elif [ -f /usr/bin/pacman ]; then
    CFLAGS+=" -DARCH"
    detect_distribution "/usr/include"
    echo -e "Detected Arch-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/emerge ]; then
    detect_distribution "/usr/include"
    CFLAGS+=" -DGENTOO"
    echo -e "Detected Gentoo-based distribution: ${GREEN}OK${NC}"
else
    echo -e "${RED}Distribution unsupported${NC}"
fi

# Check for Flatpak
if [ -f /usr/bin/flatpak ]; then
    CFLAGS+=" -DFLATPAK"
fi

# Write results to config.mk
echo "CFLAGS = ${CFLAGS}" > config.mk
echo "LDFLAGS = ${LDFLAGS}" >> config.mk