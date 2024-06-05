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
typedef unsigned long cpuInfo;
typedef const char* cpuProperty;

int Detectos();

int getProcessInfo(pid_t pid);

int LinuxSecurityModule();
// structure that stores memory information
struct MemInfo {
    double total_mem;
    double free_mem;
    double buf_cache_mem;
    double used_mem;
};
//for storing processes mountpoints in extra/storage.c
struct Mountpoints {
    char *boot;
    char *root;
    char *efi;
    char *efi_boot;
};
//for accessing function defined in extra/storage.c
int storage(void);
//function for memory calculation implemented in extra_func.c
unsigned long long extract_value(const char* line);
double round_to_nearest_tenth(double value);
#endif