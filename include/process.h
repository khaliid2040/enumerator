#ifndef PROCESS_H 
#define PROCESS_H
#include "../main.h"


//structure filled with data specific to process id
// Function to get and print process info
typedef struct {
    unsigned long utime;
    unsigned long stime;
    unsigned long cutime;
    unsigned long cstime;
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
    char cgroup[64];
    double total_cpu_time;
    double cpu_time_percent;
    double user_mode_percent;
    double system_mode_percent;
    int uid,euid,ruid;
    int gid,egid,rgid;
} ProcessInfo;

int process_file(char *path,char *filename);
void getProcessInfo(pid_t pid);

int is_pid_directory(const char *name);

//total cpu time
void process_cpu_time(void);

#endif //PROCESS_H
