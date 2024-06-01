#ifndef MAIN_H
#define MAIN_H
#define SIZE 1024 // size often used for file buffers
// including libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
typedef unsigned long cpuInfo;
typedef const char* cpuProperty;

int Detectos();

int getProcessInfo(pid_t pid);

int LinuxSecurityModule();
#endif