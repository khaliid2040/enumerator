#ifndef MAIN_H
#define MAIN_H
#define SIZE 1024 // size often used for file buffers
#define MAX_LINE_LENGTH 1024 //needs update in the project to be combined SIZE and MAX_LINE_LENGTH
//for ansi escape codes to get colors

#define ANSI_COLOR_RED     "\x1b[31m" //red
#define ANSI_COLOR_GREEN   "\x1b[32m"//green
#define ANSI_COLOR_LIGHT_GREEN "\x1b[36m"    
#define ANSI_COLOR_YELLOW  "\x1b[33m"//yellow
#define ANSI_COLOR_BLUE    "\x1b[34m"//blue
#define ANSI_COLOR_MAGENTA "\x1b[35m"//magenta
#define ANSI_COLOR_CYAN    "\x1b[36m"//cyan
#define ANSI_COLOR_RESET   "\x1b[0m"//reset the colors

//let the user decide which color to print for user preference
#ifdef RED
#define DEFAULT_COLOR    "\x1b[31m" // Red
#elif defined(YELLOW)
#define DEFAULT_COLOR    "\x1b[33m" // Yellow
#elif defined(GREEN)
#define DEFAULT_COLOR    "\x1b[32m" // Green
#elif defined(MAGENTA)
#define DEFAULT_COLOR     "\x1b[35m" // Magenta
#else 
#define DEFAULT_COLOR    "\x1b[36m" // Cyan
#endif  
// including libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syscall.h>
#include <math.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <ctype.h>
#ifdef APPARMOR 
#include <sys/apparmor.h>
#endif
#ifdef SELINUX
#include <selinux/selinux.h>
#endif  
typedef unsigned long cpuInfo;
typedef const char* cpuProperty;
typedef unsigned long page_t;
#define GiB (1024 * 1024 * 1024)
//for pci.h
#ifdef LIBPCI
#include <pci/pci.h>
#endif
//for efi.h
#ifdef LIBEFI
#include <efivar/efivar.h>
#endif
//only x86(64)
#if defined(__x86_64__) || defined(__i386__)    
#include <cpuid.h>          
#define supported
#endif
int GetSecureBootStatus(void);
void gpu_info();
//structure to fill parsed fields in /proc/cpuinfo 
struct Cpuinfo {
    char vendor[13];   
    unsigned int model;
    unsigned int family;
    int stepping;
    char model_name[64];  
};
int process_file(char *path,char *filename);
void getProcessInfo(pid_t pid);
//structure filled with data specific to process id
// Function to get and print process info
typedef struct {
    unsigned long utime;
    unsigned long stime;
    unsigned long cutime;
    unsigned long cstime;
    unsigned int priority;
    int voluntary_ctxt_switches;
    int nonvoluntary_ctxt_switches;  
    char comm[256];
    char state;
    unsigned long total_mem;
    unsigned long resident_mem;
    unsigned long shared_mem;
    unsigned long dirty_mem;
    int thread_count;
    char cgroup[64];
    double total_cpu_time;
    double cpu_time_percent;
    double user_mode_percent;
    double system_mode_percent;
    int uid,euid,ruid;
    int gid,egid,rgid;
} ProcessInfo;
int is_pid_directory(const char *name);
void LinuxSecurityModule(void);
//total cpu time
void process_cpu_time(void);
//process number of cores and processors
bool count_processor(int* cores, int* processors);
//for accessing function defined in extra/storage.c
void get_pci_info(void);
void storage(void);
//for accessng memory_info defined in memory.c
int memory_info(void);
//function for memory calculation implemented in extra_func.c
//for cpu defined in extra/cpuinfo.c
int cpuinfo(void);  
struct freq {
    unsigned long max_freq;
    unsigned long min_freq;
    unsigned long base_freq;
};
//network functions implemented in network.c
void network(void);
//for routing
void route(void);
//parsing and resding arp
void arp(void);
//used by extra/system.c
typedef struct {
    char bios_vendor[SIZE];
    char release[9];
    char date[15];
    char version[10];
    char product_name[SIZE];
    char product_family[SIZE];
    char sys_vendor[SIZE];
    char chassis_vendor[SIZE];
} System_t;
void system_enum(void);
//used in extra_fun.c
struct acpi {
    char type[10];
    char state[10];
    float temp;
    struct acpi *next;
};
void acpi_info(void);
//extra/package.c
void package_manager();
#endif