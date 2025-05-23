#ifndef  PAGER_H
#define  PAGER_H
#include "../main.h"
#include <sys/wait.h>
#include <wait.h>

extern FILE *_pager;

static inline void stop_pager() {
    if (_pager) {
        pclose(_pager);
        _pager = NULL;
    }
    exit(0);
}
/**
 * @brief Determine if the pager will be invoked based on the arguments.
 * @param argv Pointer to the argument after skipping argv[0].
 * @return true if the pager should be invoked, false otherwise.
 */
static  __attribute__ ((noinline)) bool is_pager_needed(const char* argv) {
    if (argv == NULL) return false; // No argument means we don't need pager
    if (!strcmp(argv, "-h")) return false; // Help option doesn't need the pager
    if (!strncmp(argv, "-p", 2)) return false; // -p option doesn't need the pager
    return true; // For all other cases, the pager is needed
}

void start_pager();
#endif // PAGER_H