#ifndef UTILS_H
#define UTILS_H
#include "../main.h"
#define LENGTH 1024 //used by get_pci_info
/*
    functions implemented on extra/utils.c

*/
int process_file(char *path,char *filename);

//used by main.c:Systeminfo
int is_pid_directory(const char *name);

//load specified library on library argument and search for symbol then modify the 
//pointer that is being pointed to point executable entry of the function then return the handle
//obtained from dlopen
void* load_library(const char *library, const char* symbol, void **function);

//determine if i directory is empty extensively used on sysfs directories
bool is_directory_empty(const char *path);

#ifdef LIBPCI
void gpu_info(char *model,char *vendor,size_t len);
void get_pci_info(void);
#endif

//find the specified vendor and device and return the string corresponding to that id
//parsed from pci.ids caller should free the memory
char* find_device_name(const char *vendor_id, const char *device_id);
#endif //UTILS_H