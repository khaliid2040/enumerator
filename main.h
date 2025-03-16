#ifndef MAIN_H
#define MAIN_H
#define _GNU_SOURCE
#define SIZE 256 // size often used for file buffers
#define MAX_LINE_LENGTH 1024 //needs update in the project to be combined SIZE and MAX_LINE_LENGTH
//for ansi escape codes to get colors

#define ANSI_COLOR_RED     "\x1b[31m" //red
#define ANSI_COLOR_GREEN   "\x1b[32m"//green
#define ANSI_COLOR_LIGHT_GREEN "\x1b[36m"    
#define ANSI_COLOR_YELLOW  "\x1b[33m"//yellow
#define ANSI_COLOR_BLUE    "\33[m\33[1m\33[34m"//blue
#define ANSI_COLOR_MAGENTA "\x1b[35m"//magenta
#define ANSI_COLOR_CYAN    "\x1b[36m"//cyan
#define ANSI_COLOR_RESET   "\x1b[0m"//reset the colors

//let the user decide which color to print for user preference
#ifdef RED
#define DEFAULT_COLOR    "\33[m\33[1m\33[31m" // Red
#elif defined(YELLOW)
#define DEFAULT_COLOR    "\x1b[33m" // Yellow
#elif defined(GREEN)
#define DEFAULT_COLOR    "\33[m\33[1m\33[32m" // Green
#elif defined(MAGENTA)
#define DEFAULT_COLOR     "\33[m\33[1m\33[35m" // Magenta
#else 
#define DEFAULT_COLOR    "\33[m\33[1m\33[36m" // Cyan
#endif  

#define MAX_PATH 96
#define UNIT_SIZE 1024.0 // each unit is 1024 in binary prefixes
#define HT_SMT 0x10000000 // hyperthreading on intel and/or simulatanous multi threading
// including libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <syscall.h>
#include <math.h>
#include <dirent.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <ctype.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>
#include <errno.h>
#ifdef APPARMOR 
#include <sys/apparmor.h>
#endif
#ifdef SELINUX
#include <selinux/selinux.h>
#endif  
//#include <include/common.h>
typedef unsigned long page_t;
#define GiB (1024 * 1024 * 1024)
//for pci.h
#ifdef LIBPCI
#include <pci/pci.h>
#endif
//only x86(64)
#if defined(__x86_64__) || defined(__i386__)    
#include <cpuid.h>          
#define supported
#endif

// local headers in include directory
//#include "include/common.h"
#include "system/cpuinfo.h"
#include "system/pci.h"
#include "os/process.h"
#include "os/desktop.h"
#include "system/system.h"
#include "utils/utils.h"

int GetSecureBootStatus(void);
void LinuxSecurityModule();

void storage(void);
long long get_disk_size(const char* device);

//for accessng memory_info defined in memory.c
int memory_info(void);

//network functions implemented in network.c
void network(void);
//for routing
void route(void);
//parsing and resding arp
void arp(void);

//extra/package.c
void package_manager();

// calculate size dynamically 
//caller must set unit to KiB and supply units as KiB
static inline double convert_size_unit(double size, char* unit, size_t len) {
    double converted_size = size;
    const char *units[4] = {"KiB", "MiB", "GiB", "TiB"};
    memset(unit, 0, len);

    for (int i = 0; i < 4; i++) {
        if (converted_size < UNIT_SIZE) {
            strncpy(unit, units[i], len - 1);
            break;
        }
        converted_size /= UNIT_SIZE;
    }
    return converted_size;
}
//get number of cores and processors
static inline bool count_processor(int* cores, int* processors) {
    unsigned int nprocessors = get_nprocs();
    unsigned int eax,ebx,ecx,edx;
    *processors = nprocessors; // in logical processors(threads) we done here
    #if defined(__x86_64__) || defined(__i386__) // at least do not do anything on other architectures
    __get_cpuid(0x1,&eax,&ebx,&ecx,&edx);
        if (ecx & HT_SMT)
            *cores = nprocessors / 2; // currently as i know hyperthreading feature achieved 2 threads per physical core
    #endif
        return true;
}
#endif