#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <ctype.h>
#include <grp.h>    
#include "main.h"
/*system information function*/
void systeminfo(void)
{
    //now getting the hostname and kernel information from using utsname same as kernel retrival method
    struct utsname *kernel= malloc(sizeof(struct utsname));
    
    if (uname(kernel) != -1) {
        printf(ANSI_COLOR_LIGHT_GREEN "Hostname:\t" ANSI_COLOR_RESET "%s\n",kernel->nodename);
        printf(ANSI_COLOR_LIGHT_GREEN "Kernel: \t"ANSI_COLOR_RESET "%s %s %s\n",kernel->sysname, kernel->release,kernel->machine    );
    }
    free(kernel);
    //now checking for loaded modules
    char *line=NULL;
    size_t len= 0;
    FILE *modules= fopen("/proc/modules","r");
    unsigned int count=0;
    while (getline(&line,&len,modules) != -1) {
        count++;
    }
    printf(ANSI_COLOR_LIGHT_GREEN "Loaded modules:\t" ANSI_COLOR_RESET "%u\n",count);
    fclose(modules);
    free(line); 
    //checking whether the firmware is UEFI or BIOS
    printf(ANSI_COLOR_LIGHT_GREEN "Firmware:\t" ANSI_COLOR_RESET);
    if (access("/sys/firmware/efi",F_OK) != -1) {
        printf("UEFI:\t");
        #ifdef LIBEFI
        GetSecureBootStatus();
        #endif
        
    } else {
        printf("BIOS\n");
    }
    char *env, *de;
    if (env = getenv("XDG_SESSION_TYPE")) {
        printf(ANSI_COLOR_LIGHT_GREEN "\nSession Type:\t" ANSI_COLOR_RESET "%s\n",env);
    }

    #ifdef supported // if the architecture is x86 both 32 bit and 64
    unsigned int eax,ebx,ecx,edx;
    char brand[50];
    for (int i = 0; i < 3; ++i) {
        __get_cpuid(0x80000002 + i, &eax, &ebx, &ecx, &edx);
        memcpy(brand + i * 16, &eax, 4);
        memcpy(brand + i * 16 + 4, &ebx, 4);
        memcpy(brand + i * 16 + 8, &ecx, 4);
        memcpy(brand + i * 16 + 12, &edx, 4);
    }
    brand[48] = '\0';
    printf(ANSI_COLOR_LIGHT_GREEN "CPU: \t\t"ANSI_COLOR_RESET "%s\n",brand);
    #endif
    //used for determining user home directory
    __uid_t uid= getuid();
    struct passwd *pwd=getpwuid(uid);
    char version[7];
    if (de = getenv("XDG_CURRENT_DESKTOP")) {
        printf(ANSI_COLOR_LIGHT_GREEN "Desktop:\t" ANSI_COLOR_RESET "%s ",de);
        
        if (!strcmp(de,"KDE")) {
            char path[40],*contents=NULL;
            snprintf(path,sizeof(path),"/home/%s/.config/plasma-welcomerc",pwd->pw_name);
            FILE *deVersion= fopen(path,"r");
            if (deVersion !=NULL) {
            
                size_t len= sizeof(contents);
                while (getline(&contents,&len,deVersion) != -1) {
                    sscanf(contents + 16, "%s", version);  
                }
                printf("%s\n",version);
                fclose(deVersion);
                free(contents);
            } else {
                //if the file doesn't exit print new line character
                printf("\n");
            }
        } else if(!strcmp(de,"GNOME")) {
            char version[5],*contents=NULL;
            FILE *gnome= popen("gnome-shell --version","r");
            if (gnome !=NULL) {
                size_t len= 0;
                if (getline(&contents,&len,gnome) != -1) {
                    sscanf(contents + 12, "%s",version);
                }
                printf("%s\n",version);
                pclose(gnome);
                free(contents);
            } else {
                //if the file doesn't exit print new line character
                printf("\n");
            }
        } else {
            printf(ANSI_COLOR_RED "Unsupported\n"ANSI_COLOR_RESET); 
        }
    }
    /*using the information provided by sysinfo library data structure*/
    struct sysinfo system_info;
    if (sysinfo(&system_info) == 0)
    {
        long uptime_sec = system_info.uptime; // Uptime in seconds
        long hours = uptime_sec / 3600; // Extracting hours
        long minutes = (uptime_sec % 3600) / 60; // Extracting minutes
        printf(ANSI_COLOR_LIGHT_GREEN "Uptime:\t\t" ANSI_COLOR_RESET "%ld hour%s and %ld minute%s\n", 
           hours, (hours != 1) ? "s" : "", 
           minutes, (minutes != 1) ? "s" : "");
        
    }
    //information about battery get from sysfs
    char capacity[5];
    FILE *fp= fopen("/sys/class/power_supply/BAT0/capacity","r");
    if (fp !=NULL) {
        if (fgets(capacity,sizeof(capacity),fp) != NULL) {
            size_t len= strlen(capacity);
            char status[20];
            capacity[len -1]= '\0';
            FILE *state= fopen("/sys/class/power_supply/BAT0/status","r");
            if (state !=NULL) {
                if (fgets(status,sizeof(status),state) != NULL) {
                    len=strlen(status);
                    status[len - 1]='\0';
                }
                printf(ANSI_COLOR_LIGHT_GREEN "Battery:\t"ANSI_COLOR_RESET "%s%%",capacity);
                if (!strcmp(status,"Discharging")) {

                    printf(ANSI_COLOR_YELLOW " %s"ANSI_COLOR_RESET,status);
                } else if(!strcmp(status,"Charging")) {
                    printf(ANSI_COLOR_GREEN " %s"ANSI_COLOR_RESET,status);
                } else {
                    printf(" %s\n",status);
                }
                fclose(state);
            }
        }
        fclose(fp);
    }
    struct dirent *entry;
    DIR *proc_dir = opendir("/proc");
    int num_processes = 0;
    int num_threads = 0;

    if (proc_dir == NULL) {
        perror("opendir");
        return;
    }

    // First pass: Count the number of processes
    while ((entry = readdir(proc_dir)) != NULL) {
        if (is_pid_directory(entry->d_name)) {
            num_processes++;
        }
    }

    // Reset directory stream to count threads
    rewinddir(proc_dir);
    struct dirent *task_entry;
    // Second pass: Count the number of threads
    while ((entry = readdir(proc_dir)) != NULL) {
        if (is_pid_directory(entry->d_name)) {
            char task_path[256];
            snprintf(task_path, sizeof(task_path), "/proc/%s/task", entry->d_name);
            DIR *task_dir = opendir(task_path);
            if (task_dir != NULL) {
                while ((task_entry = readdir(task_dir)) != NULL) {
                    if (is_pid_directory(task_entry->d_name)) {
                        num_threads++;
                    }
                }
                closedir(task_dir);
            }
        }
    }

    closedir(proc_dir);

    printf(ANSI_COLOR_LIGHT_GREEN "\nprocesses: " ANSI_COLOR_RESET "%d\t" ANSI_COLOR_LIGHT_GREEN "threads: " ANSI_COLOR_RESET "%d\n", num_processes,num_threads);
    /* since the program is doing system related things for security reasons it must not be run as root
    if next time needed we will remove this code but now it must be their for security reasons */
    uid= getuid();
    __gid_t gid= getgid();
    pwd;
    printf(ANSI_COLOR_YELLOW "Getting users...\n" ANSI_COLOR_RESET);
    printf("User\tUID\tGID\tShell\n");
    while ((pwd= getpwent()) != NULL) {
        if (pwd->pw_uid==0 || pwd->pw_uid >=1000) {   //also we need to print root user if it exists because it                                      // it is a normal user which can be used after
            if (strcmp(pwd->pw_name,"nobody")) {  //we don't want nobody user because it is not a normal user 
                printf("%s\t%d\t%d\t%s\n",pwd->pw_name,pwd->pw_uid,pwd->pw_gid,pwd->pw_shell);
            }
        }
    }
    endpwent();
    struct group *grp;
    //now for groups
    printf(ANSI_COLOR_YELLOW "retrieving groups...\n" ANSI_COLOR_RESET);
    printf("Group\tGID\tMembers\n");
    while ((grp=getgrent()) != NULL) {
        //same as users before we need to retrieve every group except system groups 
        //also we may need wheel and sudo groups    
        if (grp->gr_gid==0 || grp->gr_gid > 1000 || strcmp(grp->gr_name,"wheel") == 0|| strcmp(grp->gr_name,"sudo")==0  ) {
            printf("%s\t%d\t",grp->gr_name,grp->gr_gid);
            for (int i=0; grp->gr_mem[i] != NULL;i++) {
                printf("%s\t",grp->gr_mem[i]);
            }
            printf("\n");
        }
    }
    endgrent();
    printf(ANSI_COLOR_YELLOW "getting processor information\n" ANSI_COLOR_RESET);
    
    /*
        cpuinfo_buffer holds the buffer of the cpuinfo file
        buffer_size is the size of the buffer
        processors and cores are strings searched in  the file
        
    */
    int cores=0,processors=0;
    if (count_processor(&cores,&processors)) {
        printf(ANSI_COLOR_LIGHT_GREEN "cores:\t" ANSI_COLOR_RESET "%d\n",cores / 2);
        printf(ANSI_COLOR_LIGHT_GREEN "processors:\t"ANSI_COLOR_RESET "%d\n",processors);    
    }
    //checking for Linux Security Modules
    // the function is implemented in extra_func.c
    printf(ANSI_COLOR_YELLOW "Getting security modules...\n"    ANSI_COLOR_RESET);
    LinuxSecurityModule();
    
}
int main(int argc, char *argv[])
{
        printf(ANSI_COLOR_GREEN "system enumeration\n" ANSI_COLOR_RESET);
        int opt,p_value = 0,H_flag = 0,N_flag= 0,P_flag=0;
        // Parse command line options
        while ((opt = getopt(argc, argv, "p:Hnh")) != -1) {
            switch (opt) {
                case 'p':
                    P_flag=1;
                    p_value = atoi(optarg);
                    break;
                case 'H':
                    H_flag= 1;
                    break;
                  case 'n':
                    N_flag= 1;
                    break;
                case '?': // Handle unknown options
                    if (optopt == 'p')
                        printf(ANSI_COLOR_RED "Option -%c requires an argument.\n" ANSI_COLOR_RESET, optopt);
                    else if (isprint(optopt))
                        fprintf(stderr, ANSI_COLOR_RED "Unknown option `-%c'.\n" ANSI_COLOR_RESET, optopt);
                    else
                        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                    return 1;
                    break;
                case 'h':
                    printf("-p      get supplied process id information\n");
                    printf("-H      get hardware information\n");
                    printf("-n      get network information\n");
                    return 0;
                default:
                    abort();
            }
        }
        // If -p is specified
        if (P_flag) {
            process_cpu_time();
            getProcessInfo(p_value);
        } 
        // If only -H is specified
        else if (H_flag) {
        printf(ANSI_COLOR_YELLOW "Getting basic information...\n" ANSI_COLOR_RESET);
        system_enum();
        cpuinfo();
        #ifdef LIBPCI
        printf(ANSI_COLOR_YELLOW "Getting PCI information..\n" ANSI_COLOR_RESET);
        get_pci_info();
        #endif
        printf(ANSI_COLOR_YELLOW "Getting sensor information..\n"ANSI_COLOR_RESET);
        acpi_info();
        printf(ANSI_COLOR_YELLOW "\nGetting disk layout...\n" ANSI_COLOR_RESET);
        storage();
        printf(ANSI_COLOR_YELLOW "Getting memory information\n" ANSI_COLOR_RESET);
        memory_info();
        } else if(N_flag) {
            printf(ANSI_COLOR_YELLOW "Getting network information\n" ANSI_COLOR_RESET);
            network();
            printf(ANSI_COLOR_YELLOW "Getting route information...\n" ANSI_COLOR_RESET);
            route();
            printf(ANSI_COLOR_YELLOW "checking for arp entries..\n" ANSI_COLOR_RESET);
            arp();
        }
        // If no options are specified
        else {
            
            systeminfo();
        }

        return 0;
}