#!/bin/bash

LDFLAGS=""
CFLAGS="-march=native -O2 -pipe -I."
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Libraries to check
LIBS=("math" "apparmor" "selinux")
LIBPCI="libpci"

# Check for the standard libraries
for LIB in "${LIBS[@]}"; do
    if ldconfig -p | grep -q "$LIB"; then
        if [ "$LIB" == "math" ]; then
            CFLAGS+=" -DMATH_H"
            LDFLAGS+=" -lm"
            echo -e "checking ${LIB}: ${GREEN}OK${NC}"
        elif [ "$LIB" == "apparmor" ] && [ -f /usr/include/sys/apparmor.h ]; then
            CFLAGS+=" -DAPPARMOR_H"
            LDFLAGS+=" -lapparmor"
            echo -e "checking ${LIB}: ${GREEN}OK${NC}"
        elif [ "$LIB" == "selinux" ]; then
            if [ -f /usr/include/selinux/selinux.h ]; then
                CFLAGS+=" -DSELINUX_H"
                LDFLAGS+=" -lselinux"
                echo -e "checking ${LIB}: ${GREEN}OK${NC}"
            else
                echo -e "checking ${LIB}: ${RED}NO (header not found)${NC}"
            fi
        else
            echo -e "checking ${LIB}: ${RED}NO${NC}"
        fi
    else
        echo -e "checking ${LIB}: ${RED}NO${NC}"
    fi
done

libpci_path() {
    local path="$1"
    # Check for libpci independently
    if ldconfig -p | grep -q "$LIBPCI" && [ -d "${path}/pci" ]; then
        CFLAGS+=" -DLIBPCI"
        LDFLAGS+=" -lpci"
        echo -e "checking ${LIBPCI}: ${GREEN}OK${NC}"
    else
        echo -e "checking ${LIBPCI}: ${RED}NO${NC}"
    fi
}

# Detect distribution
if [ -f /usr/bin/apt ]; then
    CFLAGS+=" -DDEBIAN"
    libpci_path "/usr/include/x86_64-linux-gnu"
    echo -e "Detected Debian-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/dnf ]; then
    CFLAGS+=" -DREADHAT"
    echo -e "Detected Redhat-based distribution: ${GREEN}OK${NC}"
elif [ -f /usr/bin/pacman ]; then
    CFLAGS+=" -DARCH"
    echo -e "Detected Arch-based distribution: ${GREEN}OK${NC}"
else
    echo -e "${RED}Distribution unsupported${NC}"
fi

# Write results to config.mk
echo "CFLAGS = ${CFLAGS}" > config.mk
echo "LDFLAGS = ${LDFLAGS}" >> config.mk