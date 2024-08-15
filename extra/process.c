#include "../main.h"
#include <fcntl.h>

void Total_cpu_time(void) {
    FILE *fp;
    char line[MAX_LINE_LENGTH];

    // Open /proc/stat file
    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return;
    }

    // Variables to store CPU times
    unsigned long long user_ticks, nice_ticks, system_ticks, idle_ticks;
    unsigned long long iowait_ticks, irq_ticks, softirq_ticks, steal_ticks;
    unsigned long long guest_ticks, guest_nice_ticks;

    // Read each line and extract CPU times
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "cpu ", 4) == 0) {
            sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &user_ticks, &nice_ticks, &system_ticks, &idle_ticks,
                   &iowait_ticks, &irq_ticks, &softirq_ticks, &steal_ticks,
                   &guest_ticks, &guest_nice_ticks);

            // Calculate total CPU ticks
            unsigned long long total_ticks = user_ticks + nice_ticks + system_ticks +
                                             idle_ticks + iowait_ticks + irq_ticks +
                                             softirq_ticks + steal_ticks +
                                             guest_ticks + guest_nice_ticks;

            // Calculate CPU usage percentages
            double user_percent = (double)user_ticks / total_ticks * 100.0;
            double nice_percent = (double)nice_ticks / total_ticks * 100.0;
            double system_percent = (double)system_ticks / total_ticks * 100.0;
            double idle_percent = (double)idle_ticks / total_ticks * 100.0;
            double iowait_percent = (double)iowait_ticks / total_ticks * 100.0;
            double irq_percent = (double)irq_ticks / total_ticks * 100.0;
            double softirq_percent = (double)softirq_ticks / total_ticks * 100.0;
            double steal_percent = (double)steal_ticks / total_ticks * 100.0;
            double guest_percent = (double)guest_ticks / total_ticks * 100.0;
            double guest_nice_percent = (double)guest_nice_ticks / total_ticks * 100.0;

            // Print CPU usage percentages in the desired format
            printf(" %6.2f   %5.2f   %6.2f   %6.2f   %6.2f   %6.2f   %6.2f   %6.2f\n",
                   user_percent, nice_percent, system_percent,
                   iowait_percent, steal_percent, idle_percent,
                   irq_percent, softirq_percent);

            break; // Break after processing the first "cpu" line
        }
    }

    // Close file
    fclose(fp);
}
void process_cpu_time(void) {
    printf(ANSI_COLOR_YELLOW "System utilization\n" ANSI_COLOR_RESET);
    printf(ANSI_COLOR_BLUE "%6s   %5s   %6s   %6s   %6s   %6s   %6s   %6s\n",
           "%user", "%nice", "%system", "%iowait", "%steal", "%idle", "%irq", "%softirq" ANSI_COLOR_RESET);
    Total_cpu_time();
}

// Function to get and print process info
void getProcessInfo(int pid) {
    // Read uptime from /proc/uptime
    printf(ANSI_COLOR_YELLOW "Getting process info...\n" ANSI_COLOR_RESET);
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
        printf(ANSI_COLOR_RED "Process %d not found\n" ANSI_COLOR_RESET, pid);
        return;
    }

    // Variables to store stat values
    unsigned long utime, stime, cutime, cstime;
    char comm[256], state;
    
    // Read and parse the stat file to extract process state
    int fields_read = fscanf(statFile, "%*d (%[^)])  %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %lu %lu %lu %lu",
                             comm, &state, &utime, &stime, &cutime, &cstime);

    fclose(statFile);

    if (fields_read != 6) {
        perror("Error reading /proc/<pid>/stat");
        return;
    }

    // Calculate total CPU time in seconds
    long hertz = sysconf(_SC_CLK_TCK);
    double total_cpu_time = (utime + stime) / (double) hertz;

    // Calculate CPU time percentage compared to uptime
    double cpu_time_percent = (total_cpu_time / uptime) * 100;

    // Breakdown percentages
    double user_mode_percent = (utime / (double) hertz) / total_cpu_time * 100;
    double system_mode_percent = (stime / (double) hertz) / total_cpu_time * 100;

    // Process state description
    const char *state_string;
    switch (state) {
        case 'S':
            state_string = "sleeping";
            break;
        case 'R':
            state_string = "running";
            break;
        case 'Z':
            state_string = "zombie";
            break;
        case 'T':
            state_string = "stopped";
            break;
        case 'D':
            state_string = "disk sleep";
            break;
        default:
            state_string = "unknown";
            break;
    }

    // Get memory information from /proc/<pid>/statm
    char memPath[256];
    snprintf(memPath, sizeof(memPath), "/proc/%d/statm", pid);
    FILE *memFile = fopen(memPath, "r");
    if (memFile == NULL) {
        perror("Error opening /proc/<pid>/statm");
        return;
    }

    unsigned long total, resident, shared, dirty;
    if (fscanf(memFile, "%lu %lu %lu %*lu %*lu %*lu %lu", &total, &resident, &shared, &dirty) < 4) {
        perror("Error reading /proc/<pid>/statm");
        fclose(memFile);
        return;
    }
    fclose(memFile);
    //get number of threads 
    char thPath[256];
    struct dirent *entry;
    unsigned int count_threads=0;
    snprintf(thPath,sizeof(thPath),"/proc/%d/task",pid);
    DIR *tasks= opendir(thPath);
    if (thPath==NULL) {
        perror("Error");
        return;
    }
    while ((entry= readdir(tasks)) != NULL) {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) {
            continue;
        }
        count_threads++;
    }
    closedir(tasks);
    // Convert pages to KiB
    long page_size = sysconf(_SC_PAGE_SIZE) / 1024; // Page size in KiB
    total *= page_size;
    shared *= page_size;
    resident *= page_size;
    dirty *= page_size;

    printf(ANSI_COLOR_LIGHT_GREEN "Process ID:\t\t\t\t"ANSI_COLOR_RESET "%d\n", pid);
    printf(ANSI_COLOR_LIGHT_GREEN "Process Name:\t\t\t\t"ANSI_COLOR_RESET "%s\n", comm);
    printf(ANSI_COLOR_LIGHT_GREEN "Process State:\t\t\t\t"ANSI_COLOR_RESET "%s\n", state_string);
    printf(ANSI_COLOR_LIGHT_GREEN "Process Threads:\t\t\t" ANSI_COLOR_RESET "%u\n",count_threads);
    printf(ANSI_COLOR_LIGHT_GREEN "Total CPU Time:\t\t\t\t"ANSI_COLOR_RESET "%.2f seconds\n", total_cpu_time);
    printf(ANSI_COLOR_LIGHT_GREEN "CPU Time Percentage:\t\t\t"ANSI_COLOR_RESET "%.2f %%\n", cpu_time_percent);
    printf(ANSI_COLOR_LIGHT_GREEN "User Mode CPU Time Percentage:\t\t"ANSI_COLOR_RESET "%.2f %%\n", user_mode_percent);
    printf(ANSI_COLOR_LIGHT_GREEN "System Mode CPU Time Percentage:\t"ANSI_COLOR_RESET "%.2f %%\n", system_mode_percent);
    printf(ANSI_COLOR_YELLOW "Getting process virtual address\n" ANSI_COLOR_RESET);
    printf("%-16s%-16s%-16s%-16s\n", "Total(KiB)", "Shared(KiB)", "Resident(KiB)", "Dirty(KiB)");  
    printf("%-16d%-16d%-16d%-16d\n",total,shared,resident       ,dirty); 
}
