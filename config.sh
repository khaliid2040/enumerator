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

# Letting the user choose a theme to compile with
if [ "$1" == "--color" ]; then
    if [[ -v THEMES[$2] ]]; then
        CFLAGS+=" ${THEMES[$2]}"
        echo -e "Theme $2: ${GREEN}OK${NC}"
    else
        CFLAGS+="-DDEFAULT"
        echo -e "Theme $2: ${RED}Unsupported${NC}"
        echo -e "${YELLOW}Warning: color defaulting to green light${NC}"
    fi
fi
# enable debugging by tunning compiler debugging optimization
# and debugging symbols
# Enable debugging by tuning compiler debugging optimization and debugging symbols
if [ "$1" == "--debug" ] || [ "$1" == "-d" ]; then 
    echo -e "Debugging: ${GREEN}ON${NC}"
    CFLAGS+=" -Og -g -DDEBUG"
    if [ "$2" == "--sanitize" ] || [ "$2" == "-s" ]; then
        CFLAGS+=" -fsanitize=address"
        LDFLAGS+=" -fsanitize=address"
        echo -e "Sanitizer: ${GREEN}ON${NC}"
    else 
        echo -e "Sanitizer: ${RED}OFF${NC}"
    fi
else 
    CFLAGS+=" -O2"
    LDFLAGS+=" -flto" # Link time optimization
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

# detect selinux differently
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
