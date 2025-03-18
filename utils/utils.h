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

extern FILE* _pager;
static inline void stop_pager() {
    if (_pager) {
        pclose(_pager);
        _pager = NULL;
    }
}

void start_pager();
/**
 * @brief Checks if the current process is being debugged.
 * @return true if the process is being debugged, false otherwise.
 */
bool is_debugger_present();
//used by main.c:Systeminfo
int is_pid_directory(const char *name);

/**
 * @brief load specified library into program address space
 * @param library path to the library
 * @param symbol symbol to load from the library
 * @param function pointer to the function
 * @return handle to the library if successful, NULL otherwise, caller should call dlclose to free resources
 */
void* load_library(const char *library, const char* symbol, void **function);

/**
 * @brief Check if a directory is empty
 * @param path path to the directory
 * @return true if the directory is empty, false otherwise
 */
bool is_directory_empty(const char *path);

#ifdef LIBPCI
void gpu_info(char *model,char *vendor,size_t len);
void get_pci_info(void);
#endif

#endif //UTILS_H