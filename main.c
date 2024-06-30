#include <sys/sysinfo.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <ctype.h>
#include "main.h"
/*system information function*/
void systeminfo()
{
    //now getting the hostname and kernel information from using utsname same as kernel retrival method
    struct utsname *kernel= malloc(sizeof(struct utsname));
    
    if (uname(kernel) != -1) {
        printf("Hostname: %s\n",kernel->nodename);
        printf("%s %s\n",kernel->sysname, kernel->release);
    }
    free(kernel);
    //checking whether the firmware is UEFI or BIOS
    printf("Firmware: ");
    if (access("/sys/firmware/efi",F_OK) != -1) {
        printf("UEFI: ");
        FILE *secure_boot= fopen("/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c","r");
        char sb_buf[48];
        if (secure_boot != NULL) {
            if (fgets(sb_buf, sizeof(sb_buf),secure_boot) == NULL) {
                printf(ANSI_COLOR_GREEN "Secure Boot enabled\n" ANSI_COLOR_RESET);
            } else {
                printf(ANSI_COLOR_RED "Secure Boot disabled\n" ANSI_COLOR_RESET);
            }
        }
        fclose(secure_boot);
    } else {
        printf("BIOS\n");
    }
    char *env, *de;
    if (env = getenv("XDG_SESSION_TYPE")) {
        printf("Session Type: %s\n",env);
    }
    if (de = getenv("XDG_CURRENT_DESKTOP")) {
        printf("Desktop: %s\n",de);
    }
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
        
    }
    /* since the program is doing system related things for security reasons it must not be run as root
    if next time needed we will remove this code but now it must be their for security reasons */
    __uid_t uid= getuid();
    __gid_t gid= getgid();

    if (uid < 1000 && gid < 1000)
    {
        printf(ANSI_COLOR_RED "the program doesn't allowed to be run under this users\n" ANSI_COLOR_RESET);
        _exit(2);
    } else {
        char *username= getlogin();
        printf(ANSI_COLOR_YELLOW "\nchecking users up to 2000\n" ANSI_COLOR_RESET);
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
    printf(ANSI_COLOR_YELLOW "getting processor information\n" ANSI_COLOR_RESET);
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
    printf(ANSI_COLOR_YELLOW "checking for Linux Security Modules\n" ANSI_COLOR_RESET);
    // the function is implemented in extra_func.c
    LinuxSecurityModule();
    
}
    int main(int argc, char *argv[])
    {
        printf(ANSI_COLOR_GREEN "system enumeration\n" ANSI_COLOR_RESET);
        
        int opt;
        int p_value = 0;
        int H_flag = 0;
        // Parse command line options
        while ((opt = getopt(argc, argv, "p:Hh")) != -1) {
            switch (opt) {
                case 'p':
                    p_value = atoi(optarg);
                    break;
                case 'H':
                    H_flag= 1;
                    break;
                case '?': // Handle unknown options
                    if (optopt == 'p')
                        printf(ANSI_COLOR_RED "Option -%c requires an argument.\n" ANSI_COLOR_RESET, optopt);
                    else if (isprint(optopt))
                        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                    else
                        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                    return 1;
                    break;
                case 'h':
                    printf("-p      get supplied process id information\n");
                    printf("-H      get hardware information\n");
                    return 0;
                default:
                    abort();
            }
        }

        // If -p is specified
        if (p_value) {
            process_cpu_time();
            getProcessInfo(p_value);
        } 
        // If only -H is specified
        else if (H_flag) {
        // printf("Option -m specified.\n");
        cpuinfo();
        printf(ANSI_COLOR_YELLOW "\nGetting disk layout...\n" ANSI_COLOR_RESET);
        storage();
        printf(ANSI_COLOR_YELLOW "Getting memory information\n" ANSI_COLOR_RESET);
        memory_info();
        }
        // If no options are specified
        else {
            //printf("No options specified.\n");
            systeminfo();
        }

        return 0;
    }