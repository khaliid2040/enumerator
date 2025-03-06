#ifndef UTILS_H
#define UTILS_H
#include "../main.h"
#include <sys/wait.h>
#include <wait.h>

#define LENGTH 1024 //used by get_pci_info
/*
    functions implemented on extra/utils.c

*/
int process_file(const char *path, const char *filename);

/**
 * @brief Checks if the current process is being debugged.
 * @return true if the process is being debugged, false otherwise.
 */
bool is_debugger_present();
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

#endif //UTILS_H