#include "../main.h"
#include <fcntl.h>
#include <sys/resource.h>

static void Total_cpu_time(void) {
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
/*
    The below functions are specific to process with process id
*/
static int readUptime(double *uptime, double *idletime) {
    FILE *file = fopen("/proc/uptime", "r");
    if (!file) return -1;
    if (fscanf(file, "%lf %lf", uptime, idletime) != 2) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

// getpriority syscall to get the actual priority not nice value which glibc wrapper returns
static inline int process_getpriority(unsigned int which,unsigned int who) {
    return syscall(SYS_getpriority,which,who);
}
/*get the command of the current process and it's parent 
 *processInfo structure must be filled
 * extract ppid from processInfo->ppid
 */
static bool get_comm_processes(int pid , ProcessInfo *info) {
    char path[MAX_PATH];
    FILE *fp;
    snprintf(path,MAX_PATH,"/proc/%d/comm",pid);
    fp = fopen(path,"r");
    if (!fp)
        return false;
    if (fgets(info->comm,sizeof(info->comm),fp) == NULL) {
        fclose(fp);
        return false;
    }
    fclose(fp);
    // parent command now
    snprintf(path,MAX_PATH,"/proc/%d/comm",info->ppid);
    fp = fopen(path,"r");
    if (!fp)
        return false;
    if (fgets(info->pcomm,sizeof(info->pcomm),fp) == NULL) {
        fclose(fp);
        return false;
    }
    info->pcomm[strlen(info->pcomm) - 1] = '\0';
    fclose(fp);
    return true;
}

static int readProcessStats(int pid, ProcessInfo *info) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *file = fopen(path, "r");          
    if (!file) return -1;  // File could not be opened

    // Read the process stats. Make sure the format matches the actual format of /proc/[pid]/stat.
    int fields_read = fscanf(file, "%*d %*s %c %d %*d %*d %*d %*d %*u %u %*u %u %lu %lu %lu %lu",
                             &info->state, &info->ppid, &info->minflt, &info->majrflt,&info->utime, &info->stime, 
                             &info->cutime, &info->cstime);
    info->priority = process_getpriority(PRIO_PROCESS,pid);
    fclose(file);
    if (!get_comm_processes(pid,info))
        return 1;
    // Check if all expected fields were successfully read.
    return (fields_read == 8) ? 0 : -1;
}  
static int readMemoryInfo(int pid, ProcessInfo *info) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/statm", pid);
    FILE *file = fopen(path, "r");
    if (!file) return -1;
    if (fscanf(file, "%lu %lu %lu %*lu %*lu %*lu %lu", &info->total_mem, 
               &info->resident_mem, &info->shared_mem, &info->dirty_mem) < 4) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}
static int countThreads(int pid) {
    char *content=NULL,path[MAX_PATH];
    unsigned int Threads = 0;
    FILE *fp;
    size_t size = 0;
    snprintf(path,MAX_PATH,"/proc/%d/status",pid);
    fp = fopen(path,"r");
    if (!fp)
        return -1;
    while (getline(&content,&size,fp) != -1) {
        if (!strncmp(content,"Threads:",8)) {
            if (!sscanf(content,"Threads: %u",&Threads) != 1) {
                break;
            }
        }
    }
    return Threads;
}
static int readCgroup(int pid, char *cgroup) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/cgroup", pid);
    FILE *file = fopen(path, "r");
    if (!file) return -1;
    if (fgets(cgroup, 64, file) == NULL) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}
static int get_ctxt_switches(ProcessInfo *info,int pid) {
    char path[64],lines[64];
    snprintf(path,sizeof(path),"/proc/%d/status",pid);
    FILE *fp= fopen(path,"r");
    if (fp==NULL) {
        return 1;
    }
    while (fgets(lines,sizeof(lines),fp) != NULL) {
        if (strncmp(lines, "voluntary_ctxt_switches:", 24) == 0) {
            if (sscanf(lines, "voluntary_ctxt_switches: %d", &info->voluntary_ctxt_switches) != 1) {
                fprintf(stderr, "Error parsing voluntary context switches\n");
                fclose(fp);
                return EXIT_FAILURE;
            }
        }
        if (strncmp(lines, "nonvoluntary_ctxt_switches:", 27) == 0) {
            if (sscanf(lines, "nonvoluntary_ctxt_switches: %d", &info->nonvoluntary_ctxt_switches) != 1) {
                fprintf(stderr, "Error parsing nonvoluntary context switches\n");
                fclose(fp);
                return EXIT_FAILURE;
            }
        }
    }   
    fclose(fp);
    return 0;
}
// get real, effected user id return 2 if succeed
static void get_uid_gid(ProcessInfo *info,int pid) {
    char path[64],contents[64];
    snprintf(path,sizeof(path),"/proc/%d/status",pid);
    FILE *fp = fopen(path,"r");
    if (!fp) {
        return;
    }
    while (fgets(contents,sizeof(contents),fp) != NULL) {
        if (!strncmp(contents, "Uid:", 4)) {
            // Parse Uid: line, which might have tabs or spaces
            if (sscanf(contents, "Uid:%d\t%d\t%d", &info->uid,&info->euid,&info->ruid) == 3) {
                continue;
            } 
        }
		if (!strncmp(contents,"Gid:",4)) {
			if (sscanf(contents,"Gid:%d\t%d\t%d\t", &info->gid, &info->egid,&info->rgid) != 3) {
				continue;
			}
			break;
		}
    }
}
static void calculateCPUInfo(ProcessInfo *info, double uptime) {
    long hertz = sysconf(_SC_CLK_TCK);
    info->total_cpu_time = (info->utime + info->stime) / (double) hertz;
    info->cpu_time_percent = (info->total_cpu_time / uptime) * 100;
    info->user_mode_percent = (info->utime / (double) hertz) / info->total_cpu_time * 100;
    info->system_mode_percent = (info->stime / (double) hertz) / info->total_cpu_time * 100;
}

static void printProcessInfo(const ProcessInfo *info,int pid) {
    long page_size = sysconf(_SC_PAGE_SIZE) / 1024; // Page size in KiB
    char unit[4][4];
    unsigned long total = convert_size_unit(info->total_mem * page_size,unit[0],4);
    unsigned long shared = convert_size_unit(info->shared_mem * page_size,unit[1],4);
    unsigned long resident = convert_size_unit(info->resident_mem * page_size,unit[2],4);
    unsigned long dirty = convert_size_unit(info->dirty_mem * page_size,unit[3],4);

    const char *state_string;
    switch (info->state) {
        case 'S': state_string = "sleeping"; break;
        case 'R': state_string = "running"; break;
        case 'Z': state_string = "zombie"; break;
        case 'T': state_string = "stopped"; break;
        case 'D': state_string = "disk sleep"; break;
        default: state_string = "unknown"; break;
    }

    printf(DEFAULT_COLOR "Process ID:\t\t\t\t" ANSI_COLOR_RESET "%d\n", pid);
    printf(DEFAULT_COLOR "Process Name:\t\t\t\t" ANSI_COLOR_RESET "%s", info->comm);
    printf(DEFAULT_COLOR "Process State:\t\t\t\t" ANSI_COLOR_RESET "%s\n", state_string);
    printf(DEFAULT_COLOR "Process Threads:\t\t\t" ANSI_COLOR_RESET "%d\n", info->thread_count);
    printf(DEFAULT_COLOR "Parent process:\t\t\t\t"ANSI_COLOR_RESET "%d(%s)\n",info->ppid,info->pcomm);
    printf(DEFAULT_COLOR "priority:\t\t\t\t"ANSI_COLOR_RESET "%d\n",info->priority);
    printf(DEFAULT_COLOR "Cgroup slice:\t\t\t\t" ANSI_COLOR_RESET "%s", info->cgroup);
    printf(DEFAULT_COLOR "Uid/euid/uid\t\t\t\t"ANSI_COLOR_RESET "%u\t%u\t%u\t\n",info->uid,info->euid,info->ruid);
    printf(DEFAULT_COLOR "Gid/egid/rgid\t\t\t\t" ANSI_COLOR_RESET "%u\t%u\t%u\t\n" ,info->gid,info->egid,info->rgid);
    printf(DEFAULT_COLOR "Total CPU Time:\t\t\t\t" ANSI_COLOR_RESET "%.2f seconds\n", info->total_cpu_time);
    printf(DEFAULT_COLOR "major/minor page faults:\t\t"ANSI_COLOR_RESET "%d/%d\n",info->majrflt,info->minflt);
    printf(DEFAULT_COLOR "Context switches:\t\t\t"ANSI_COLOR_RESET "voluntary=%d nonvoluntary=%d\n",info->voluntary_ctxt_switches,info->nonvoluntary_ctxt_switches);
    printf(DEFAULT_COLOR "CPU Time Percentage:\t\t\t" ANSI_COLOR_RESET "%.2f %%\n", info->cpu_time_percent);
    printf(DEFAULT_COLOR "User Mode CPU Time Percentage:\t\t" ANSI_COLOR_RESET "%.2f %%\n", info->user_mode_percent);
    printf(DEFAULT_COLOR "System Mode CPU Time Percentage:\t" ANSI_COLOR_RESET "%.2f %%\n", info->system_mode_percent);
    printf(ANSI_COLOR_YELLOW "Getting process virtual address\n" ANSI_COLOR_RESET);
    printf("%s\t\t%s\t\t%s\t%s\n", "Total", "Shared", "Resident", "Dirty");
    printf("%lu %s\t\t%lu %s\t\t%lu %s\t\t%lu %s\n", total, unit[0], shared, unit[1], resident, unit[2], dirty, unit[3]);


}
void getProcessInfo(int pid) {
    printf(ANSI_COLOR_YELLOW "Getting process info...\n" ANSI_COLOR_RESET);

    ProcessInfo info = {0};
    double uptime, idletime;

    if (readUptime(&uptime, &idletime) != 0) {
        perror("Error opening /proc/uptime");
        return;
    }
    int stat = readProcessStats(pid,&info);
    if (stat == -1) {
        fprintf(stderr,ANSI_COLOR_RED "process %d not found\n"ANSI_COLOR_RESET,pid);
        return;
    } else if (stat == 1) {
        fprintf(stderr,"failed to get parent process command\n");
    }

    if (readMemoryInfo(pid, &info) != 0) {
        perror("Error reading /proc/<pid>/statm");
        return;
    }

    info.thread_count = countThreads(pid);
    if (info.thread_count < 0) {
        perror("Error counting threads");
        return;
    }

    if (readCgroup(pid, info.cgroup) != 0) {
        perror("Error reading cgroup");
        return;
    }
    if (get_ctxt_switches(&info,pid)) {
        fprintf(stderr,"error reading /proc/%d/status",pid);
        return;
    }
    get_uid_gid(&info,pid);
    calculateCPUInfo(&info, uptime);
    printProcessInfo(&info,pid);
}
