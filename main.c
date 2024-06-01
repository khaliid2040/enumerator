#include <sys/sysinfo.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include "main.h"
/*system information function*/
void systeminfo()
{
    //calling the detector function to check running operating system
    Detectos();
    //now getting the hostname from /etc/hostname
    char hostname_buf[256];
    FILE *hostname= fopen("/etc/hostname","r");
    if (hostname == NULL) {
        perror("failed to open /etc/hostname");
    }
    else {
        fgets(hostname_buf,256,hostname);
        printf("hostname: %s",hostname_buf);
    }
    fclose(hostname);
    /*using the information provided by sysinfo library data structure*/
    struct sysinfo system_info;
    if (sysinfo(&system_info) == 0)
    {
        long uptime_sec = system_info.uptime; // Uptime in seconds
        long hours = uptime_sec / 3600; // Extracting hours
        long minutes = (uptime_sec % 3600) / 60; // Extracting minutes
        printf("Uptime: %ld hour%s and %ld minute%s", 
           hours, (hours != 1) ? "s" : "", 
           minutes, (minutes != 1) ? "s" : "");
        printf("\nTotal memory: %d GB\n", system_info.totalram / 1024 / 1024 / 1024);
        
    }
    /* since the program is doing system related things for security reasons it must not be run as root
    if next time needed we will remove this code but now it must be their for security reasons */
    __uid_t uid= getuid();
    __gid_t gid= getgid();
    //now getting kernel information
    printf("checking running kernel\n");
    struct utsname kernel_info;
    if (uname(&kernel_info) == -1)
    {
        perror("error");
    }
    printf("kernel version: %s\n",kernel_info.release);
    printf("Warning: this script wan't supposed to be run under the root user \n");
    if (uid < 1000 && gid < 1000)
    {
        printf("the program doesn't allowed to be run under this users\n");
        _exit(2);
    } else {
        char *username= getlogin();
        printf("checking users up to 2000\n");
        for (int i=0; i < 2000; i++)
        {
            struct passwd *pw= getpwuid(i);
            if (i<1000 && i !=0)
            {
                continue;
            } else
            {
                if (pw !=NULL)
                {
                    printf("user %s exist\n", pw->pw_name);
                }
            }
        }
    }
    printf("getting process information\n");
   // long total_time= system_info.uptime;
    //getProcessInfo(getpid, total_time);
    
    /*
        cpuinfo_buffer holds the buffer of the cpuinfo file
        buffer_size is the size of the buffer
        processors and cores are strings searched in  the file
        
    */
    char *cpuinfo_buffer= NULL;
    size_t buffer_size= 0;
    cpuProperty processors = "processor";
    cpuProperty cores = "cores";
    int processors_count= 0;
    int cores_count= 0;
    FILE *cpuinfo = fopen("/proc/cpuinfo","r");

    if (cpuinfo == NULL) {
        printf("Failed to open cpuinfo file.\n");

    }
    while (getline(&cpuinfo_buffer, &buffer_size, cpuinfo) != -1)
    {
        if (strstr(cpuinfo_buffer, processors) != NULL) {
            processors_count++;
        }
        if (strstr(cpuinfo_buffer, cores) != NULL) {
            cores_count++;
        }
    }
    printf("Number of cores: %d\n", cores_count /2);
    printf("number of processors: %d\n",processors_count);
    // checking firmware

    free(cpuinfo_buffer);
    fclose(cpuinfo);
    //checking for Linux Security Modules
    printf("checking for Linux Security Modules\n");
    // the function is implemented in extra_func.c
    LinuxSecurityModule();
}
int main(int argc, char *argv[])
{
    printf("system enumeration\n");
    
    
    if (argc < 2)
    {
        systeminfo();
        return 1;
    } else if (strcmp(argv[1], "-p")== 0)
    {
        int pid = atoi(argv[2]);
        getProcessInfo(pid);
    } else {
        printf("invalid option\n");
        return 2;
    }
    
    return 0;
}