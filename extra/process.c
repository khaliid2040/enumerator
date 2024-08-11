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
void getProcessInfo(int pid) {
    // Read uptime from /proc/uptime
    printf(ANSI_COLOR_YELLOW "Getting process..\n"ANSI_COLOR_RESET);
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
                                        "%*lu %*lu %*lu %*lu %  lu %lu %lu %lu %lu",
                                        comm, &state, &utime, &stime, &cutime,&cstime);

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
    //now getting memory information of the process via /proc/pid/statm
    char path[30];
    char mem_buf[30];
    size_t total,shared,residual,dirty;
    page_t tpage,shpage,rpage,dpage;
    snprintf(path,30,"/proc/%d/statm",pid);
    FILE *mem= fopen(path,"r");
    //whether fopen and fgets fail or not we need to continue execution 
    if (mem == NULL) {
        perror("fopen ");
    }
    int succeed;
    if ((succeed=fscanf(mem,"%d%d%d%*d%*d%*d%d",&tpage,&rpage,&shpage,&dpage)) < 4) {
        perror("fscanf");
    }
    total= tpage * 4;
    shared= shpage * 4;
    residual= rpage *4;
    dirty= dpage * 4;
    // Print results
    printf("Process ID: %d\n", pid);
    printf("Process Name: %s\n", comm);
    printf("Process State: %s\n", state_string);
    printf("Total CPU Time: %.2f seconds\n", total_cpu_time);
    printf("CPU Time Percentage: %.2f %%\n", cpu_time_percent);
    printf("User Mode CPU Time Percentage: %.2f %%\n", user_mode_percent);
    printf("System Mode CPU Time Percentage: %.2f %%\n", system_mode_percent);
    printf("I/O Wait CPU Time Percentage: %.2f %%\n", io_wait_percent);
    printf(ANSI_COLOR_YELLOW "Getting process virtual address\n" ANSI_COLOR_RESET);
    printf("%-16s%-16s%-16s%-16s\n", "Total(KiB)", "Shared(KiB)", "Resident(KiB)", "Dirty(KiB)");  
    printf("%-16d%-16d%-16d%-16d\n",total,shared,residual,dirty); 
}

