#ifndef MAIN_H
#define MAIN_H
#define SIZE 1024 // size often used for file buffers

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
//function for memory calculation implemented in extra_func.c
unsigned long long extract_value(const char* line);
double round_to_nearest_tenth(double value);
#endif