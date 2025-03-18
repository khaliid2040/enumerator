#include "../main.h"

FILE* _pager = NULL;

static FILE *open_pager() {
    char command[64];
    const char *pager = getenv("PAGER");
    if (!pager) {
        pager = "less"; // Default to less if PAGER environment variable is not set
    }
    snprintf(command, sizeof(command), "%s -R", pager);
    FILE *pipe = popen(command, "w");
    if (!pipe) {
        perror("popen");
        return NULL;
    }

    return pipe;
}

void start_pager() {
    FILE *pager = open_pager();
    if (!pager) {
        return;
    }
    _pager = pager;

    // print the initial message inside the pager
    fprintf(pager, ANSI_COLOR_GREEN "System enumeration\n" ANSI_COLOR_RESET);
    fflush(pager);
    // Redirect stdout to the pager
    int pager_fd = fileno(pager);
    if (pager_fd == -1) {
        perror("fileno");
        pclose(pager);
        return;
    }

    if (dup2(pager_fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        pclose(pager);
        return;
    }
}