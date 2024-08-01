#ifndef MAIN_H
#define MAIN_H
#define SIZE 1024 // size often used for file buffers
#define MAX_LINE_LENGTH 1024 //needs update in the project to be combined SIZE and MAX_LINE_LENGTH
//for ansi escape codes to get colors

#define ANSI_COLOR_RED     "\x1b[31m" //red
#define ANSI_COLOR_GREEN   "\x1b[32m"//green
#define ANSI_COLOR_YELLOW  "\x1b[33m"//yellow
#define ANSI_COLOR_BLUE    "\x1b[34m"//blue
#define ANSI_COLOR_MAGENTA "\x1b[35m"//magenta
#define ANSI_COLOR_CYAN    "\x1b[36m"//cyan
#define ANSI_COLOR_RESET   "\x1b[0m"//reset the colors
// including libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
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

int process_file(char *path,char *filename);
void getProcessInfo(pid_t pid);

void LinuxSecurityModule(void);
//total cpu time
void process_cpu_time(void);
//process number of cores and processors
bool count_processor(int* cores, int* processors);
//for accessing function defined in extra/storage.c
void storage(void);
//for accessng memory_info defined in memory.c
int memory_info(void);
//function for memory calculation implemented in extra_func.c
//for cpu defined in extra/cpuinfo.c
int cpuinfo(void);  
//network functions implemented in network.c
void network(void);
//for routing
void route(void);
//parsing and resding arp
void arp(void);
//used by extra/syste.c
typedef struct System {
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
#endif