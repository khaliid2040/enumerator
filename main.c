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
        printf("Hostname: %s\n",kernel->nodename);
        printf("%s %s\n",kernel->sysname, kernel->release);
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
    printf("Kernel modules loaded: %u\n",count);
    fclose(modules);
    free(line); 
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
            fclose(secure_boot);                    
        }
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
    struct passwd *pwd;
    printf(ANSI_COLOR_YELLOW "\nGetting users...\n" ANSI_COLOR_RESET);
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
        printf("Number of cores: %d\n",cores / 2);
        printf("Number of processors: %d\n",processors);    
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