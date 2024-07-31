#!/bin/bash
LIBS=("math" "apparmor" "selinux")
LDFLAGS=
CFLAGS="-march=native -O2 -pipe -I."
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
for lib in "${LIBS[@]}"; do
        # attempt 1
    LIB="/usr/include/${lib}.h"
    if [ -f $LIB ]; then
        echo -e "checking ${lib}:   ${GREEN}OK${NC}"
        if [ "$lib" = "math" ]; then
        LDFLAGS+="-lm"
        CFLAGS+="-DMATH_H"
        fi
        continue 
    fi
    # attempt 2
    LIB="/usr/include/sys/${lib}.h"
    if [ -f $LIB ]; then
        echo -e "checking ${lib}:   ${GREEN}OK${NC}"
        if [ "$lib" = "apparmor" ]; then
        LDFLAGS+=" -lapparmor"
        CFLAGS+=" -DAPPARMOR_H"
        fi
        continue
    fi
    LIB="/usr/include/selinux/${lib}.h"
    if [ -f $LIB ]; then
    echo -e "checking ${lib}:    ${GREEN}OK${NC}"
    LDFLAGS+=" -lselinux"
    CFLAGS+=" -DSELINUX_H"
    continue
    fi
    #attempt three and final one 
    LIB="/usr/include/${lib}/${lib}.h"
    if [ -f $LIB ]; then
        echo -e "checking ${lib}:   ${GREEN}OK${NC}"
    else
        echo -e "checking ${lib}: ${RED}NO${NC}"
    fi 
done
echo "LDFLAGS=${LDFLAGS}" > config.mk
echo "CFLAGS=${CFLAGS}" >> config.mk