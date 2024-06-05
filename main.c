#include <sys/sysinfo.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <ctype.h>
#include "main.h"
/*system information function*/
void systeminfo()
{
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
        printf("\nTotal memory: %d GB", system_info.totalram / 1024 / 1024 / 1024);
        printf(ANSI_COLOR_YELLOW " Note: for more info use -m\n" ANSI_COLOR_RESET);
        
    }
    /* since the program is doing system related things for security reasons it must not be run as root
    if next time needed we will remove this code but now it must be their for security reasons */
    __uid_t uid= getuid();
    __gid_t gid= getgid();
    //now getting kernel information
    printf("\nchecking running kernel\n");
    struct utsname kernel_info;
    if (uname(&kernel_info) == -1)
    {
        perror("error");
    }
    printf("kernel version: %s\n",kernel_info.release);
    printf(ANSI_COLOR_YELLOW "Warning: this script wan't supposed to be run under the root user \n" ANSI_COLOR_RESET);
    if (uid < 1000 && gid < 1000)
    {
        printf(ANSI_COLOR_RED "the program doesn't allowed to be run under this users\n" ANSI_COLOR_RESET);
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
    printf("getting processor information\n");
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
int memory_info() {
    FILE *fp;
    char buffer[255];
    unsigned long long total_mem = 0, free_mem = 0, buffer_cache = 0;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    // Iterate over each line in the file
    while (fgets(buffer, 255, fp)) {
        // Check for the lines containing required information
        if (strstr(buffer, "MemTotal:") != NULL) {
            total_mem = extract_value(buffer);
        } else if (strstr(buffer, "MemFree:") != NULL) {
            free_mem = extract_value(buffer);
        } else if (strstr(buffer, "Buffers:") != NULL || strstr(buffer, "Cached:") != NULL) {
            // Both "Buffers" and "Cached" contribute to buffer/cache
            buffer_cache += extract_value(buffer);
        }
    }

    fclose(fp);

    // Convert memory sizes to gibibytes and round to the nearest tenth
    double total_mem_gib = round_to_nearest_tenth(total_mem / (1024.0 * 1024.0));
    double free_mem_gib = round_to_nearest_tenth(free_mem / (1024.0 * 1024.0));
    double buffer_cache_gib = round_to_nearest_tenth(buffer_cache / (1024.0 * 1024.0));
    double used_mem_gib = round_to_nearest_tenth(total_mem_gib - free_mem_gib - buffer_cache_gib);

    struct MemInfo memory;
    memory.total_mem= total_mem_gib;
    memory.free_mem= free_mem_gib;
    memory.buf_cache_mem= buffer_cache_gib;
    memory.used_mem= used_mem_gib;
    // Output the results
    printf("Total Memory: %.1f GiB\n", memory.total_mem);
    printf("Free Memory: %.1f GiB\n", memory.free_mem);
    printf("Buffer/Cache: %.1f GiB\n", memory.buf_cache_mem);
    printf("Used Memory: %.1f GiB\n", memory.used_mem);
}
int main(int argc, char *argv[])
{
    printf(ANSI_COLOR_GREEN "system enumeration\n" ANSI_COLOR_RESET);
    
     int opt;
    int p_value = 0;
    int m_flag = 0;

    // Parse command line options
    while ((opt = getopt(argc, argv, "p:m:h")) != -1) {
        switch (opt) {
            case 'p':
                p_value = atoi(optarg);
                break;
            case 'm':
                m_flag = 1;
                break;
            case '?': // Handle unknown options
                if (optopt == 'p')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
                break;
            case 'h':
                printf("-p      get supplied process id information\n");
                printf("-m      get memory information\n");
                return 0;
            default:
                abort();
        }
    }

    // If both options are specified
    if (p_value != 0 && m_flag) {
       // printf("Both options specified: -p %d and -m.\n", p_value);
       getProcessInfo(p_value);
       memory_info();
    }
    // If only -p is specified
    else if (p_value != 0) {
        //printf("Option -p specified with value: %d.\n", p_value);
        getProcessInfo(p_value);
    }
    // If only -m is specified
    else if (m_flag) {
       // printf("Option -m specified.\n");
       systeminfo();
       printf("\n");
       memory_info();
    }
    // If no options are specified
    else {
        //printf("No options specified.\n");
        systeminfo();
    }

    return 0;
}