#ifndef CPUINFO_H
#define CPUINFO_H
#include "../main.h"

//structure to fill parsed fields in /proc/cpuinfo 
struct Cpuinfo {
    unsigned int processor;
    unsigned int implementer;
    unsigned int architecture;
    unsigned int variant;
    unsigned int part;
    unsigned int revision;
    char features[48];
};



struct freq {
    unsigned long max_freq;
    unsigned long min_freq;
    unsigned long base_freq;
};


void cpuinfo(void);  

// system/cpuinfo.c and system/virt.c sharing same header

//everything about virtualization and containerization
//used by system/virt.c
typedef enum {
    KVM,
    Virtualbox,
    Vmware,
    hyperv,
    xen,
    docker,
    podman,
    none,
    unknown
} Virtualization;
Virtualization detect_hypervisor();
//detect if we run on vm or not if we run return true if not return false
static inline bool is_hypervisor_present() {
    unsigned int eax,ebx,ecx,edx;
    #if defined(__x86_64__) || defined(__i386__)
    __get_cpuid(1,&eax,&ebx,&ecx,&edx);
    if (ecx & (1U << 31)) return true;
    #endif
    return false;
}
#endif // CPUINFO_H