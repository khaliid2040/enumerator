#ifndef PROCESS_H 
#define PROCESS_H
#include "../main.h"
#include <time.h>

//structure filled with data specific to process id
// Function to get and print process info
typedef struct {
    unsigned long utime;
    unsigned long stime;
    unsigned long starttime;
    unsigned long minflt; //minor page fault
    unsigned long majrflt; // major page fault
    unsigned int ppid;
    unsigned int priority;
    int voluntary_ctxt_switches;
    int nonvoluntary_ctxt_switches;  
    char comm[256];
    char pcomm[256]; // parent /proc/[pid]/comm
    char state;
    unsigned long total_mem;
    unsigned long resident_mem;
    unsigned long shared_mem;
    unsigned long dirty_mem;
    int thread_count;
    char cgroup[120];
    double total_cpu_time;
    char time_unit[10]; //either holds milliseconds or seconds depending on cpu being higher than 1.0 or lower respectively
    double cpu_time_percent;
    double user_mode_percent;
    double system_mode_percent;
    int uid,euid,ruid;
    int gid,egid,rgid;
} ProcessInfo;

struct cpu_times {
    unsigned long long user_ticks;
    unsigned long long nice_ticks;
    unsigned long long system_ticks;
    unsigned long long idle_ticks;
    unsigned long long iowait_ticks;
    unsigned long long irq_ticks;
    unsigned long long softirq_ticks;
    unsigned long long steal_ticks;
    unsigned long long guest_ticks;
    unsigned long long guest_nice_ticks;
    unsigned long total_ticks;
};
int process_file(char *path,char *filename);
void getProcessInfo(int pid,unsigned int interval);

int is_pid_directory(const char *name);

//total cpu time
void process_cpu_time(void);

#endif //PROCESS_H
