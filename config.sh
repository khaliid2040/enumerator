#!/bin/bash

LDFLAGS=""
CFLAGS="-march=native -O2 -pipe -I."
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[33m'
NC='\033[0m'

# letting the user choose a theme to compile with
if [ "$1" == "--color" ]; then
case $2 in
    red)
        CFLAGS+=" -DRED"
        echo -e "Theme $2: ${GREEN}OK${NC}"
    ;;
    green)
        CFLAGS+=" -DGREEN"
        echo -e "Theme $2: ${GREEN}OK${NC}"
    ;;
    yellow)
        CFLAGS+=" -DYELLOW"
        echo -e "Theme $2: ${GREEN}OK${NC}"
    ;; 
    magenta)
        CFLAGS+=" -DMAGENTA"
        echo -e "Theme $2: ${GREEN}OK${NC}"
        ;;
    *)
        CFLAGS+="-DDEFALT"
        echo -e "Theme $2: ${RED}Unsupported${NC}"
        echo -e "${YELLOW}Warning: color defaulting green light${NC}"
esac
fi

# Libraries to check
LIBS=("math" "apparmor" "selinux" "efivar" "blkid" "libudev" "libsensors")
LIBPCI="pci"

# Check for the standard libraries
for LIB in "${LIBS[@]}"; do
    if [ "$LIB" == "math" ]; then
        if ldconfig -p | grep -q "libm.so"; then
            CFLAGS+=" -DMATH_H"
            LDFLAGS+=" -lm"
            echo -e "checking ${LIB}: ${GREEN}OK${NC}"
        else
            echo -e "checking ${LIB}: ${RED}NO${NC}"
        fi
    elif [ "$LIB" == "apparmor" ] && [ -f /usr/include/sys/apparmor.h ]; then
        CFLAGS+=" -DAPPARMOR_H"
        LDFLAGS+=" -lapparmor"
        echo -e "checking ${LIB}: ${GREEN}OK${NC}"
    elif [ "$LIB" == "selinux" ] && [ -f /usr/include/selinux/selinux.h ]; then
        CFLAGS+=" -DSELINUX_H"
        LDFLAGS+=" -lselinux"
        echo -e "checking ${LIB}: ${GREEN}OK${NC}"
    elif [ "$LIB" == "efivar" ] && [ -f /usr/include/efivar/efivar.h ]; then
        CFLAGS+=" -DLIBEFI"
        LDFLAGS+=" -lefivar"
        echo -e "checking ${LIB}: ${GREEN}OK${NC}"
    elif [ "$LIB" == "blkid" ] && [ -f /usr/include/blkid/blkid.h ]; then
        CFLAGS+=" -DBLKID"
        LDFLAGS+=" -lblkid"
        echo -e "checking ${LIB}: ${GREEN}OK${NC}"
    elif [ "$LIB" == "libudev" ] && [ -f /usr/include/libudev.h ]; then
        CFLAGS+=" -DLIBUDEV"
        LDFLAGS+=" -ludev"
        echo -e "checking ${LIB}: ${GREEN}OK${NC}"
    elif [ "$LIB" == "libsensors" ] && [ -f /usr/include/sensors/sensors.h ]; then
        CFLAGS+=" -DLIBSENSORS"
        LDFLAGS+=" -lsensors"
        echo -e "checking ${LIB}: ${GREEN}OK${NC}"
    else
        echo -e "checking ${LIB}: ${RED}NO${NC}"
    fi
done

libpci_path() {
    local path="$1"
    # Check for libpci independently
    if [ -d "${path}/pci" ]; then
        CFLAGS+=" -DLIBPCI"
        LDFLAGS+=" -lpci"
        echo -e "checking libpci: ${GREEN}OK${NC}"
    else
        echo -e "checking libpci: ${RED}NO${NC}"
    fi
}

# Detect distribution and check for libpci
if [ -f /usr/bin/apt ]; then
    CFLAGS+=" -DDEBIAN"
    libpci_path "/usr/include/x86_64-linux-gnu"
    echo -e "Detected Debian-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/dnf ]; then
    CFLAGS+=" -DREADHAT"
    libpci_path "/usr/include"
    echo -e "Detected Redhat-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/pacman ]; then
    CFLAGS+=" -DARCH"
    libpci_path "/usr/include"
    echo -e "Detected Arch-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/emerge ]; then
    libpci_path "/usr/include"
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
