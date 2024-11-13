#include <pwd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <ctype.h>
#include <grp.h>    
#include "main.h"

// get the distribution name
static void distro_name() {
    printf(DEFAULT_COLOR "Distribution:\t"ANSI_COLOR_RESET);
    FILE *file = fopen("/etc/os-release", "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    char *line=NULL;
    size_t len=0;
    while (getline(&line,&len,file) != -1) {
        if (!strncmp(line,"NAME=",5)) {
            char *name = line + 5;  // Skip "NAME="
            // Remove leading and trailing quotes
            if (name[0] == '"') name++;
            size_t len = strlen(name);
            if (name[len - 1] == '\n') name[len - 2] = '\0';
            if (name[len - 1] == '"') name[len - 1] = '\0'; 
            printf("%s\n",name);
        }
    }
    free(line);
    fclose(file);
}
/*system information functions*/
static void print_hostname_and_kernel() {
    struct utsname kernel;
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf(DEFAULT_COLOR "Hostname: \t" ANSI_COLOR_RESET "%s\n", hostname);
    }
    if (uname(&kernel) != -1) {
        printf(DEFAULT_COLOR "Kernel: \t" ANSI_COLOR_RESET "%s %s %s\n", kernel.sysname, kernel.release, kernel.machine);
    }
}

static void print_loaded_modules() {
    char *line = NULL;
    size_t len = 0;
    FILE *modules = fopen("/proc/modules", "r");
    unsigned int count = 0;
    if (modules) {
        while (getline(&line, &len, modules) != -1) {
            count++;
        }
        fclose(modules);
        printf(DEFAULT_COLOR "Loaded modules:\t" ANSI_COLOR_RESET "%u\n", count);
        free(line);
    } else {
        perror("Failed to open /proc/modules");
    }
}

static void print_firmware_info() {
    printf(DEFAULT_COLOR "Firmware:\t" ANSI_COLOR_RESET);
    if (access("/sys/firmware/efi", F_OK) != -1) {
        printf("UEFI  ");
        #ifdef LIBEFI
        GetSecureBootStatus();
        #endif
    } else {
        printf("BIOS\n");
    }
}

static void print_systemd_info() {
    char output[20];
    FILE *init = popen("systemctl --version", "r");
    if (init) {
        if (fgets(output, sizeof(output)-1, init) != NULL) {
            printf(DEFAULT_COLOR "\nInit: \t\t" ANSI_COLOR_RESET "%s)", output);
        }
        pclose(init);
    }

    int count_units = 0;
    DIR *units = opendir("/usr/lib/systemd/system");
    struct dirent *unit_entry;
    if (units) {
        while ((unit_entry = readdir(units)) != NULL) {
            if (strcmp(unit_entry->d_name, ".") != 0 && strcmp(unit_entry->d_name, "..") != 0) {
                count_units++;
            }
        }
        closedir(units);
    }
    printf(" %d units installed\n", count_units);
}

static void print_cpu_info() {
    #ifdef supported
    unsigned int eax, ebx, ecx, edx;
    char brand[50] = {0};
    for (int i = 0; i < 3; ++i) {
        __get_cpuid(0x80000002 + i, &eax, &ebx, &ecx, &edx);
        memcpy(brand + i * 16, &eax, 4);
        memcpy(brand + i * 16 + 4, &ebx, 4);
        memcpy(brand + i * 16 + 8, &ecx, 4);
        memcpy(brand + i * 16 + 12, &edx, 4);
    }
    printf(DEFAULT_COLOR "CPU: \t\t" ANSI_COLOR_RESET "%s", brand);
    int cores = 0, processors = 0;
    if (count_processor(&cores, &processors)) {
        printf(" cores: %d threads: %d\n", cores / 2, processors);
    }
    #endif
}

static void print_gpu_info() {
    #ifdef LIBPCI
    char model[32], vendor[10];
    gpu_info(model, vendor);
    printf(DEFAULT_COLOR "GPU:\t\t" ANSI_COLOR_RESET "%s %s\n", vendor, model);
    #endif
}

static void print_memory_and_uptime() {
    struct sysinfo system_info;
    if (sysinfo(&system_info) == 0) {
        printf(DEFAULT_COLOR "Memory:\t\t" ANSI_COLOR_RESET "%.1f GB\n", (float)system_info.totalram / 1024.0 / 1024.0 / 1024.0);
        long uptime_sec = system_info.uptime;
        long hours = uptime_sec / 3600;
        long minutes = (uptime_sec % 3600) / 60;
        printf(DEFAULT_COLOR "Uptime:\t\t" ANSI_COLOR_RESET "%ld hour%s and %ld minute%s\n",
               hours, (hours != 1) ? "s" : "",
               minutes, (minutes != 1) ? "s" : "");
    }
}

static void print_load_average() {
    printf(DEFAULT_COLOR "Load:\t\t" ANSI_COLOR_RESET);
    char loadbuf[48];
    float ldavg1, ldavg5, ldavg15;
    FILE *load = fopen("/proc/loadavg", "r");
    if (load) {
        if (fgets(loadbuf, sizeof(loadbuf), load) != NULL) {
            sscanf(loadbuf, "%f %f %f", &ldavg1, &ldavg5, &ldavg15);
            printf("%.2f %.2f %.2f\n", ldavg1, ldavg5, ldavg15);
        }
        fclose(load);
    } else {
        perror("failed to open /proc/loadavg");
    }
}

static void print_desktop_environment() {
        char *env, *de;
    if (env = getenv("XDG_SESSION_TYPE")) {
        printf(DEFAULT_COLOR "Session Type:\t" ANSI_COLOR_RESET "%s\n",env);
    }   
    //used for determining user home directory
    __uid_t uid= getuid();
    struct passwd *pwd=getpwuid(uid);
    char version[7];
    if (de = getenv("XDG_CURRENT_DESKTOP")) {
        printf(DEFAULT_COLOR "Desktop:\t" ANSI_COLOR_RESET "%s ",de);
        
        if (!strcmp(de,"KDE")) {
            char path[40],*contents=NULL    ;
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
}

static void print_battery_info() {
    char capacity[5];
    FILE *fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (fp) {
        if (fgets(capacity, sizeof(capacity), fp) != NULL) {
            char status[20];
            size_t len = strlen(capacity);
            capacity[len - 1] = '\0';                   
            FILE *state = fopen("/sys/class/power_supply/BAT0/status", "r");
            if (state) {
                if (fgets(status, sizeof(status), state) != NULL) {
                    len = strlen(status);
                    status[len - 1] = '\0';
                }
                printf(DEFAULT_COLOR "Battery:\t" ANSI_COLOR_RESET "%s%%", capacity);
                if (strcmp(status, "Discharging") == 0) {
                    printf(ANSI_COLOR_YELLOW " %s" ANSI_COLOR_RESET, status);
                } else if (strcmp(status, "Charging") == 0) {
                    printf(ANSI_COLOR_GREEN " %s" ANSI_COLOR_RESET, status);
                } else {
                    printf(" %s\n", status);
                }
                fclose(state);
            }
        }
        fclose(fp);
    }
}

static void print_process_and_thread_count() {
    DIR *proc_dir = opendir("/proc");
    int num_processes = 0;
    int num_threads = 0;
    if (proc_dir) {
        struct dirent *entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            if (is_pid_directory(entry->d_name)) {
                num_processes++;
            }
        }
        rewinddir(proc_dir);
        struct dirent *task_entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            if (is_pid_directory(entry->d_name)) {
                char task_path[256];
                snprintf(task_path, sizeof(task_path), "/proc/%s/task", entry->d_name);
                DIR *task_dir = opendir(task_path);
                if (task_dir) {
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
        printf(DEFAULT_COLOR "\nprocesses: " ANSI_COLOR_RESET "%d\t" DEFAULT_COLOR "threads: " ANSI_COLOR_RESET "%d\n", num_processes, num_threads);
    } else {
        perror("opendir");
    }
}

static void print_user_and_group_info() {
    __uid_t uid = getuid();
    __gid_t gid = getgid();
    struct passwd *pwd;
    printf(ANSI_COLOR_YELLOW "Getting users...\n" ANSI_COLOR_RESET);
    printf("User\tUID\tGID\tShell\n");
    while ((pwd = getpwent()) != NULL) {
        if (pwd->pw_uid == 0 || pwd->pw_uid >= 1000) {
            if (strcmp(pwd->pw_name, "nobody") != 0) {
                printf("%s\t%d\t%d\t%s\n", pwd->pw_name, pwd->pw_uid, pwd->pw_gid, pwd->pw_shell);
            }
        }
    }
    endpwent();

    struct group *grp;
    printf(ANSI_COLOR_YELLOW "retrieving groups...\n" ANSI_COLOR_RESET);
    printf("Group\tGID\tMembers\n");
    while ((grp = getgrent()) != NULL) {
        if (grp->gr_gid == 0 || grp->gr_gid > 1000 || strcmp(grp->gr_name, "wheel") == 0 || strcmp(grp->gr_name, "sudo") == 0) {
            printf("%s\t%d\t", grp->gr_name, grp->gr_gid);
            for (int i = 0; grp->gr_mem[i] != NULL; i++) {
                printf("%s\t", grp->gr_mem[i]);
            }
            printf("\n");
        }
    }
    endgrent();
}

void systeminfo() {
    distro_name();
    print_hostname_and_kernel();
    print_loaded_modules();
    print_firmware_info();
    print_systemd_info();
    print_cpu_info();
    print_gpu_info();
    print_memory_and_uptime();
    print_load_average();
    print_desktop_environment();
    print_battery_info();
    print_process_and_thread_count();
    print_user_and_group_info();
    // LinuxSecurityModule() should be called here if defined elsewhere
    printf(ANSI_COLOR_YELLOW "Getting security modules...\n" ANSI_COLOR_RESET);
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
            //get this process information if specified 0 for debugging purposes
            if (!p_value) {
                p_value = getpid();
            }
            getProcessInfo(p_value);
        } 
        // If only -H is specified
        else if (H_flag) {
        printf(ANSI_COLOR_YELLOW "Getting basic information...\n" ANSI_COLOR_RESET);
        system_enum();
        cpuinfo();
        printf(ANSI_COLOR_YELLOW "Getting memory information\n" ANSI_COLOR_RESET);
        memory_info();
        printf(ANSI_COLOR_YELLOW "\nGetting disk layout...\n" ANSI_COLOR_RESET);
        storage();
        #ifdef LIBPCI
        printf(ANSI_COLOR_YELLOW "Getting PCI information..\n" ANSI_COLOR_RESET);
        get_pci_info();
        #endif
        printf(ANSI_COLOR_YELLOW "Getting sensor information..\n"ANSI_COLOR_RESET);
        acpi_info();
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