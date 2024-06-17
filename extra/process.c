#include "../main.h"
#include <fcntl.h>

void getProcessInfo(int pid) {
    // Read uptime from /proc/uptime
    double uptime, idletime;
    FILE *uptimeFile = fopen("/proc/uptime", "r");
    if (uptimeFile == NULL) {
        perror("Error opening /proc/uptime");
        return;
    }
    fscanf(uptimeFile, "%lf %lf", &uptime, &idletime);
    fclose(uptimeFile);

    // Read /proc/<pid>/stat
    char statPath[256];
    snprintf(statPath, sizeof(statPath), "/proc/%d/stat", pid);
    FILE *statFile = fopen(statPath, "r");
    if (statFile == NULL) {
        printf(ANSI_COLOR_RED "Process %d not found\n"ANSI_COLOR_RESET, pid);
        return;
    }

    // Variables to store stat values
    unsigned long utime, stime, cutime, cstime;
    char comm[256],state;
    // Read and parse the stat file to extract process state
    int fields_read = fscanf(statFile, "%*d (%[^)]) %c %*d %*d %*d %*d %*d %*u "
                                        "%*lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu",
                                        comm, &state, &utime, &stime, &cutime, &cstime);

    fclose(statFile);

    if (fields_read != 6) {
        perror("Error reading /proc/<pid>/stat");
        return;
    }
    // Calculate total CPU time in seconds
    long hertz = sysconf(_SC_CLK_TCK);
    double total_cpu_time = (utime + stime + cutime + cstime) / (double) hertz;

    // Calculate CPU time percentage compared to uptime
    double cpu_time_percent = (total_cpu_time / uptime) * 100;

    // Calculate breakdown percentages
    double user_mode_percent = (utime / (double) hertz) / total_cpu_time * 100;
    double system_mode_percent = (stime / (double) hertz) / total_cpu_time * 100;
    double io_wait_percent = ((cutime + cstime) / (double) hertz) / total_cpu_time * 100;
    //we have to change the characters to descriptive string
    char *state_string;
    switch (state) {
        case 'S':
            state_string= "sleeping";
            break;
        case 'R':
            state_string= "Running";
            break;
        case 'Z':
            state_string= "Zombie";
            break;
        case 'T':
            state_string= "Stopped";
            break;
        case 'D':
            state_string= "Uinterruptable sleep";
            break;
        default:
            state_string= "Unknown";
            break;
    }
    // Print results
    printf("Process ID: %d\n", pid);
    printf("Process Name: %s\n", comm);
    printf("Process State: %s\n", state_string);
    printf("Total CPU Time: %.2f seconds\n", total_cpu_time);
    printf("CPU Time Percentage: %.2f %%\n", cpu_time_percent);
    printf("User Mode CPU Time Percentage: %.2f %%\n", user_mode_percent);
    printf("System Mode CPU Time Percentage: %.2f %%\n", system_mode_percent);
    printf("I/O Wait CPU Time Percentage: %.2f %%\n", io_wait_percent);
}

