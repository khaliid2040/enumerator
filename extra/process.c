#include "../main.h"
#include <fcntl.h>
#include <sys/resource.h>

static void read_global_stat(struct cpu_times *times) {
    char line[MAX_LINE_LENGTH];
    FILE *fp = fopen("/proc/stat","r");
    if (!fp) return;

    while (fgets(line,sizeof(line),fp) != NULL) {
        if (strncmp(line,"cpu ",4)) {
            sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",&times->user_ticks,
                                                                                &times->nice_ticks,
                                                                                &times->system_ticks,
                                                                                &times->idle_ticks,
                                                                                &times->iowait_ticks,
                                                                                &times->irq_ticks,
                                                                                &times->softirq_ticks,
                                                                                &times->steal_ticks,
                                                                                &times->guest_ticks,
                                                                                &times->guest_nice_ticks);
        break; //first entry only                                                                        
        }
    }
    fclose(fp);
    times->total_ticks = times->user_ticks + times->system_ticks + 
                                 times->nice_ticks + times->iowait_ticks +
                                 times->irq_ticks + times->idle_ticks +
                                 times->steal_ticks + times->guest_nice_ticks + 
                                 times->guest_ticks + times->softirq_ticks;
    
}
static void Total_cpu_time(void) {
    struct cpu_times times = {0};

   read_global_stat(&times);
    double user_percent = (double)times.user_ticks / times.total_ticks * 100.0;
    double system_percent = (double)times.system_ticks / times.total_ticks * 100.0;
    double nice_percent = (double)times.nice_ticks / times.total_ticks * 100.0;
    double idle_percent = (double)times.idle_ticks / times.total_ticks * 100.0;
    double iowait_percent = (double)times.iowait_ticks / times.total_ticks * 100.0;
    double irq_percent = (double)times.irq_ticks / times.total_ticks * 100.0;
    double softirq_percent = (double)times.softirq_ticks / times.total_ticks * 100.0;
    double steal_percent = (double)times.steal_ticks / times.total_ticks * 100.0;
    double guest_percent = (double)times.guest_ticks / times.total_ticks * 100.0;
    double guest_nice_percent = (double)times.guest_nice_ticks / times.total_ticks * 100.0;

    printf(" %6.2f   %5.2f   %6.2f   %6.2f   %6.2f   %6.2f   %6.2f   %6.2f\n",
                   user_percent, nice_percent, system_percent,
                   iowait_percent, steal_percent, idle_percent,
                   irq_percent, softirq_percent);

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
int fields_read = fscanf(file, 
    "%*d "   /*  1  - PID (skip) */ 
    "%*s "   /*  2  - Comm (process name, skip safely) */ 
    "%c "    /*  3  - State */
    "%d "    /*  4  - PPID (Parent Process ID) */
    "%*d "   /*  5  - Process Group ID (skip) */
    "%*d "   /*  6  - Session ID (skip) */
    "%*d "   /*  7  - TTY (skip) */
    "%*d "   /*  8  - TPGID (skip) */
    "%*u "   /*  9  - Flags (skip) */
    "%lu "   /* 10  - minflt (Minor Page Faults) */
    "%*u "   /* 11  - cminflt (skip) */
    "%lu "   /* 12  - majflt (Major Page Faults) */
    "%*u "   /* 13  - cmajflt (skip) */
    "%lu "   /* 14  - utime (User Mode CPU Time) */
    "%lu "   /* 15  - stime (Kernel Mode CPU Time) */
    "%*lu "   /* 16  - cutime (Children's User Mode Time) */
    "%*lu "   /* 17  - cstime (Children's Kernel Mode Time) */
    "%*ld "  /* 18  - Priority (skip) */
    "%*ld "  /* 19  - Nice Value (skip) */
    "%*ld "  /* 20  - Number of Threads (skip) */
    "%*ld "  /* 21  - ItRealValue (skip, always 0) */
    "%lu ",  /* 22  - starttime (Process start time in clock ticks) */
    &info->state, &info->ppid, 
    &info->minflt, &info->majrflt, 
    &info->utime, &info->stime,  
    &info->starttime);
    
    info->priority = process_getpriority(PRIO_PROCESS,pid);
    fclose(file);
    if (!get_comm_processes(pid,info))
        return 1;
    // Check if all expected fields were successfully read.
    return (fields_read == 7) ? 0 : -1;
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
    fclose(fp);
    free(content);
    return Threads;
}
static int readCgroup(int pid, ProcessInfo *info) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/cgroup", pid);
    FILE *file = fopen(path, "r");
    if (!file) return -1;
    if (fgets(info->cgroup, sizeof(info->cgroup), file) == NULL) {
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
    if (!fp) return;

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
    fclose(fp);
}
//either uptime or cpu_times structure should be supplied
//if both of them supplied then cpu_times will take the priority so if you want calculation to be based on
//uptime then cpu_times structure should be NULL
static void calculateCPUInfo(ProcessInfo *info, double uptime, struct cpu_times *times) {
    double (*round)(double x);
    void *handle = load_library(LIBM_SO, "round", (void**)&round);
    if (!handle) {
        fprintf(stderr, ANSI_COLOR_RED "Fatal: failed to load libm.so %s\n", dlerror());
        return;
    }

    long hertz = sysconf(_SC_CLK_TCK);
    if (hertz <= 0) hertz = 100;  // Fallback to common value

    // Total CPU time (user + system)
    info->total_cpu_time = (info->utime + info->stime) / (double)hertz;

    double process_runtime;
    
    // Case 1: No interval, use uptime (absolute time since process start)
    if (!times) {
        process_runtime = uptime - (info->starttime / (double)hertz);
    } 
    // Case 2: Interval provided, use total_ticks as precomputed CPU runtime
    else {
        process_runtime = times->total_ticks / (double)hertz;
    }

    // Calculate CPU time percentage (rounded to 2 decimals)
    if (process_runtime > 1e-9) {
        info->cpu_time_percent = (info->total_cpu_time / process_runtime) * 100;
        info->cpu_time_percent = round(info->cpu_time_percent * 100) / 100;  // Fix rounding issues
    } else {
        info->cpu_time_percent = 0;
    }

    // User/system mode split (no division by HZ needed)
    long total_ticks = info->utime + info->stime;
    if (total_ticks > 0) {
        info->user_mode_percent = (info->utime / (double)total_ticks) * 100;
        info->system_mode_percent = (info->stime / (double)total_ticks) * 100;
    } else {
        info->user_mode_percent = 0;
        info->system_mode_percent = 0;
    }
    //finally if cpu time is more than second use seconds, which is fine since it is already is.
    // But if it is less than seconds then use ms. Chnage info->time_unit respectively.
    if (info->total_cpu_time < 1.0) {
        info->total_cpu_time *= 1000.0;
        strncpy(info->time_unit,"ms",3);
    } else if (info->total_cpu_time == 1.0) {
        strncpy(info->time_unit,"second",7);
    } else {
        strncpy(info->time_unit,"seconds",8);
    }
    dlclose(handle);
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
    printf(DEFAULT_COLOR "major/minor page faults:\t\t"ANSI_COLOR_RESET "%d/%d\n",info->majrflt,info->minflt);
    printf(DEFAULT_COLOR "Context switches:\t\t\t"ANSI_COLOR_RESET "voluntary=%d nonvoluntary=%d\n",info->voluntary_ctxt_switches,info->nonvoluntary_ctxt_switches);
    printf(DEFAULT_COLOR "Total CPU Time:\t\t\t\t" ANSI_COLOR_RESET "%.2f %s\n", info->total_cpu_time,info->time_unit);
    printf(DEFAULT_COLOR "CPU Time Percentage:\t\t\t" ANSI_COLOR_RESET "%.2f %%\n", info->cpu_time_percent);
    printf(DEFAULT_COLOR "User Mode CPU Time Percentage:\t\t" ANSI_COLOR_RESET "%.2f %%\n", info->user_mode_percent);
    printf(DEFAULT_COLOR "System Mode CPU Time Percentage:\t" ANSI_COLOR_RESET "%.2f %%\n", info->system_mode_percent);
    printf(ANSI_COLOR_YELLOW "Getting process virtual address\n" ANSI_COLOR_RESET);
    printf("%s\t\t%s\t\t%s\t%s\n", "Total", "Shared", "Resident", "Dirty");
    printf("%lu %s\t\t%lu %s\t\t%lu %s\t\t%lu %s\n", total, unit[0], shared, unit[1], resident, unit[2], dirty, unit[3]);


}
static inline int interval_sleep(time_t seconds) {
    struct timespec time;
    time.tv_sec = seconds;
    time.tv_nsec = 0;
    if (nanosleep(&time,NULL) == -1) {
        fprintf(stderr,ANSI_COLOR_RED "Error: failed to pause at specified interval, %s\n"ANSI_COLOR_RESET,strerror(errno));
        return -1;
    }
    return 0;
}

static int processinfo_interval(int pid, time_t seconds, ProcessInfo *info,struct cpu_times *times) 
{
    ProcessInfo before = {0},after = {0}; // before and after sleep 
    struct cpu_times t_before = {0}, t_after = {0};
    //first take snapshot
    if (readProcessStats(pid,&before) == -1) return -1;
    read_global_stat(&t_before);
    //then pause for specified duration
    if (interval_sleep(seconds) == -1) return -1;
    //after sleep read 
    if (readProcessStats(pid,&after) == -1) return -1;
    read_global_stat(&t_after);
    //okay we are ready calculate the difference by substracting after from before
    info->utime = after.utime - before.utime;
    info->stime = after.stime - before.stime;
    times->total_ticks = t_after.total_ticks - t_before.total_ticks;
    //do not forget to copy other fields
    info->state = after.state;
    info->ppid = after.ppid;
    info->majrflt = after.majrflt;
    info->minflt = after.minflt;
    info->starttime = after.starttime;
    info->priority = process_getpriority(PRIO_PROCESS,pid);
    strncpy(info->comm,after.comm,sizeof(info->comm));
    strncpy(info->pcomm,after.pcomm,sizeof(info->pcomm));
    return 0;
}
void getProcessInfo(int pid, unsigned int interval) {
    printf(ANSI_COLOR_YELLOW "Getting process info...\n" ANSI_COLOR_RESET);

    ProcessInfo info = {0};
    struct cpu_times times = {0};
    double uptime, idletime;

    if (!interval) {
        if (readUptime(&uptime, &idletime) != 0) {
            perror("Error opening /proc/uptime");
            return;
        }
        int stat = readProcessStats(pid, &info);
        if (stat == -1) {
            fprintf(stderr, ANSI_COLOR_RED "Process %d not found\n" ANSI_COLOR_RESET, pid);
            return;
        } else if (stat == 1) {
            fprintf(stderr, "Failed to get parent process command\n");
        }
        calculateCPUInfo(&info, uptime, NULL);
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

    if (readCgroup(pid, &info) != 0) {
        perror("Error reading cgroup");
        return;
    }

    if (get_ctxt_switches(&info, pid)) {
        fprintf(stderr, "Error reading /proc/%d/status", pid);
        return;
    }

    get_uid_gid(&info, pid);

    if (interval) {
        int stat = processinfo_interval(pid, (long)interval, &info, &times);
        if (stat == -1) {
            fprintf(stderr, ANSI_COLOR_RED "Error: couldn't read process at interval %u\n" ANSI_COLOR_RESET, interval);
            return;
        } else if (stat == 1) {
            fprintf(stderr, ANSI_COLOR_YELLOW "Warning: couldn't get parent process command\n" ANSI_COLOR_RESET);
        }
        calculateCPUInfo(&info, uptime, &times);
    }

    printProcessInfo(&info, pid);
}
