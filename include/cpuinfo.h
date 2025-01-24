#ifndef CPUINFO_H
#define CPUINFO_H
#include "../main.h"

//structure to fill parsed fields in /proc/cpuinfo 
struct Cpuinfo {
    char vendor[13];   
    unsigned int model;
    unsigned int family;
    int stepping;
    char model_name[64];  
};



struct freq {
    unsigned long max_freq;
    unsigned long min_freq;
    unsigned long base_freq;
};

//function for memory calculation implemented in extra_func.c
//for cpu defined in system/cpuinfo.c
int cpuinfo(void);  

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

#endif // CPUINFO_H