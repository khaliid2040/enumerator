#include <stdio.h>
#include <string.h>
#include "main.h"
//for detecting the operating system because only one operating system supported and it is Linux
int Detectos() {
    char buffer[120];
    FILE *uname= popen("uname -s","r");
    if(uname == NULL)
    {
        perror("error running command");
    }
    if (fgets(buffer,sizeof(buffer),uname) !=NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer,"Linux")==0)
        {
            printf("detected operating system: Linux\n");
        }
        else {
            printf("not linux");
            return -1;
        }
    }
    pclose(uname);
    return 0;
}
//for getting process information like cpu time
int getProcessInfo(pid_t pid) {
    char statPath[256];
    snprintf(statPath, sizeof(statPath), "/proc/%d/stat", pid);

    FILE* statFile = fopen(statPath, "r");
    if (statFile == NULL) {
        perror("Failed to open stat file");
        return -1;
    }

    int processId;
    char processName[256];
    cpuInfo utime, stime;
    cpuInfo hertz = sysconf(_SC_CLK_TCK);
    double process_total_time;
    double cpu_percentage;

    fscanf(statFile, "%d (%[^)]) %*c %*d %*d %*d %*d %*d %*u "
                     "%*lu %*lu %*lu %*lu %*lu %lu %lu", &processId, processName, &utime, &stime);

    process_total_time = (double)(utime + stime) / hertz;
    double process_total_time_percent= (process_total_time / utime) * 100;
    double userspace_time= (process_total_time / utime);
    double system_time= (process_total_time / stime);
    printf("Process ID: %d\n", pid);
    printf("Process Name: %s\n", processName);
    printf("CPU Time: %.2f seconds\n", process_total_time);
    printf("process time percent %.2f %\n", process_total_time_percent);
    printf("user time percent %.2f %\n", userspace_time);
    printf("system time percent %.2f %\n", system_time);
    fclose(statFile);

    return 0;
}
//for checking the Linux security Modules
int LinuxSecurityModule() {
    FILE *fp;
    char buffer[SIZE];
    
    // Open the /sys/kernel/securitylsm file for reading
    fp = fopen("/sys/kernel/security/lsm", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read the contents of the file
    if (fgets(buffer, SIZE, fp) != NULL) {
        // Check if SELinux is being used
        if (strstr(buffer, "selinux")) {
            printf("SELinux detected.\n");
        } 
        // Check if AppArmor is being used
        else if (strstr(buffer, "apparmor")) {
            printf("AppArmor detected.\n");
        } 
        // If neither SELinux nor AppArmor is found, print unknown
        else {
            printf("Unknown LSM is in use.\n");
        }
    } else {
        printf("Unable to determine the LSM.\n");
    }

    // Close the file
    fclose(fp);

    return 0;

}
//this function is for memory calculation
unsigned long long extract_value(const char* line) {
    // Find the colon (':') character
    const char* colon = strchr(line, ':');
    if (colon == NULL)
        return 0;

    // Move the pointer past the colon
    colon++;

    // Extract the value using strtoull
    return strtoull(colon, NULL, 10);
}
// and this one is for round
double round_to_nearest_tenth(double value) {
    return round(value * 10.0) / 10.0;
}