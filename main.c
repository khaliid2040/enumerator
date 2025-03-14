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
            printf("%s ",name);
        }
        if (!strncmp(line,"VERSION=",8)) {
            char *version = line + 8;  // Skip "VERSION="
            // Remove leading and trailing quotes
            if (version[0] == '"') version++;
            size_t len = strlen(version);
            if (version[len - 1] == '\n') version[len - 2] = '\0';
            if (version[len - 1] == '"') version[len - 1] = '\0'; 
            printf("%s",version);
        }
    }
    printf("\n");
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
        if (GetSecureBootStatus() == -1) {
            printf(ANSI_COLOR_RED "secureboot Unsupported\n");
        }
    } else {
        printf("BIOS\n");
    }
}

static void print_systemd_info() {
    char *version= NULL;
    int units;
    
    // first check if init is systemd
    if (is_init_systemd()) {
        #ifdef SYSTEMD
        if (get_systemd_version(&version) == -1) return;
        units = get_systemd_units();
        #endif
        printf(DEFAULT_COLOR"Init:\t\t"ANSI_COLOR_RESET "systemd %s units=%d\n",version,units);
        free(version);
    }
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
        printf(" cores: %d threads: %d\n", cores, processors);
    }
    #endif
}

static void print_gpu_info() {
    FILE *fp;
    DIR *dir;
    char *device = NULL;
    char vendor_str[7], device_id_str[7], path[MAX_PATH];
    struct dirent *entry;
    long vendor_id, device_id;

    dir = opendir("/sys/class/drm");
    if (!dir) return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        // Currently, we limit to only one card
        if (!strcmp(entry->d_name, "card1") || !strcmp(entry->d_name,"card0")) {
            // Read vendor ID
            snprintf(path, sizeof(path), "/sys/class/drm/%s/device/vendor", entry->d_name);
            fp = fopen(path, "r");
            if (!fp) continue;
            if (!fgets(vendor_str, sizeof(vendor_str), fp)) {
                fclose(fp);
                continue;
            }
            fclose(fp);
            vendor_id = strtol(vendor_str, NULL, 16); // Convert to integer

            // Read device ID
            snprintf(path, sizeof(path), "/sys/class/drm/%s/device/device", entry->d_name);
            fp = fopen(path, "r");
            if (!fp) continue;
            if (!fgets(device_id_str, sizeof(device_id_str), fp)) {
                fclose(fp);
                continue;
            }
            fclose(fp);
            device_id = strtol(device_id_str, NULL, 16); // Convert to integer

            // Find device name
            device = find_device_name(vendor_str, device_id_str);
            if (!device) continue;

            printf(DEFAULT_COLOR "GPU:\t\t" ANSI_COLOR_RESET "%s %s\n",
                   vendor_id == 0x8086 ? "Intel" :
                   vendor_id == 0x10de ? "NVIDIA" :
                   vendor_id == 0x1002 ? "AMD" : "Unknown", device);

            free(device); // Free memory allocated by find_device_name()
            //break; // Only process one GPU
        }
    }

    closedir(dir);
}
static void print_memory_and_uptime() {
    struct sysinfo system_info;
    char unit[4] = {0};
    if (sysinfo(&system_info) == 0) {
        printf(DEFAULT_COLOR "Memory:\t\t" ANSI_COLOR_RESET "%.1f %s\n", (float)convert_size_unit(system_info.totalram / UNIT_SIZE,unit,4),unit);
        printf(DEFAULT_COLOR "Uptime:\t\t"ANSI_COLOR_RESET);
        long uptime_sec = system_info.uptime;
        long hours = uptime_sec / 3600;
        long minutes = (uptime_sec % 3600) / 60;
        long days = 0;
        while (hours >= 24) { days++; hours -= 24;}  // convert hours to days if hours is greater than 24 hour
        if (days) printf("%ld days ",days); //print days if only days is non zero
        if (hours) printf("%ld hour%s ",hours, (hours != 1) ? "s" : ""); // print hours only if it is non zero
        printf("%ld minute%s\n",minutes, (minutes != 1) ? "s": ""); //always print minutes even if it is zero so the field don't become empty
    }
}

static void print_disk_info() {
    FILE *fp;
    unsigned long long disk_size;
    char model[64],unit[5];
    const char *paths[] = {
        "/sys/block/nvme0n1/device/model",
        "/sys/block/sda/device/model"
    }; // we don't have any other option but to gues the disk
    const char *disks[] = {"nvme0n1", "sda"}; // also guess this one since the root disk can be either sata based or nvme
    printf(DEFAULT_COLOR"Disk:\t\t"ANSI_COLOR_RESET);
    for (int i=0; i < 2; i++) {
        disk_size = get_disk_size(disks[i]);
        if (!disk_size) continue;
        fp = fopen(paths[i],"r");
        if (!fp) continue;
        if (fgets(model,sizeof(model),fp) == NULL) {
            fclose(fp);
            continue;
        }
        model[strlen(model) - 1] = '\0'; // remove \n 
        trim_whitespace(model);
        if (strstr(paths[i],"nvme")) printf("NVMe ");
        if (strstr(paths[i],"sda")) printf("SATA ");
        printf("%s ",model);
        fclose(fp);
        break;
    }
    printf("size %.1f %s\n",convert_size_unit((float)(disk_size / UNIT_SIZE),unit,sizeof(unit)),unit);
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
    char *env, shell_info[VERSION_LEN];
    char version[VERSION_LEN];
    Desktop desktop;
    enum Shell sh;

    env = getenv("XDG_SESSION_TYPE");
    if (env) {
        printf(DEFAULT_COLOR "Session Type:\t" ANSI_COLOR_RESET "%s ", env);
        if (!strcmp(env, "wayland")) {
            #ifdef LIBWAYLAND
            detect_compositor();
            #endif
            printf("(%s)\n", (compositor == SWAY ? "Sway" :
                             compositor == KWIN ? "Kwin" :
                             compositor == MUTTER ? "Mutter" :
                             compositor == WESTON ? "Weston" : ""));
        } else {
            printf("\n");
        }
    }

    get_shell_version(shell_info, &sh);
    printf(DEFAULT_COLOR "Shell:\t\t" ANSI_COLOR_RESET "%s %s\n", (sh == Bash ? "Bash":
                                                                   sh == Zsh ? "Zsh" :
                                                                   sh == Fish ? "Fish" :
                                                                   sh == Csh ? "Csh" : ""), shell_info);
    #ifdef LIBWAYLAND
    if (env && !strcmp(env, "wayland")) {
        get_display_model(WAYLAND);
        printf(DEFAULT_COLOR "Display:\t" ANSI_COLOR_RESET "%s %dx%d %d Hz\n", out_info.make, out_info.width, out_info.height, out_info.refresh_rate / 1000);
        free(out_info.make); // allocated by get_display_model using strdup  
        free(out_info.model); // allocated by get_display_model using strdup
        free(out_info.discription); // allocated by get_display_model using strdup
    }
    #endif

    desktop = Detect_desktop(version);
    switch (desktop) {
        case GNOME:
            printf(DEFAULT_COLOR "Desktop:\t" ANSI_COLOR_RESET "Gnome %s\n", version);
            break;
        case KDE:
            printf(DEFAULT_COLOR "Desktop:\t" ANSI_COLOR_RESET "KDE %s\n", version);
            break;
        case XFCE:
            printf(DEFAULT_COLOR "Desktop:\t" ANSI_COLOR_RESET "XFCE %s\n", version);
            break;
        case MATE:
            printf(DEFAULT_COLOR "Desktop:\t" ANSI_COLOR_RESET "Mate %s", version);
            break;
        case NONE:
            break;
    }
}

static void print_locales_info() {
    char timezone[20],buffer[80],*env;
    struct tm *tm;
    time_t t;

    FILE *fp = fopen("/etc/timezone","r");
    if (!fp) return;
    if (!fgets(timezone,sizeof(timezone),fp)) {fclose(fp); return;}
    fclose(fp);
    env = getenv("LANG");
    printf(DEFAULT_COLOR"Locale:\t\t"ANSI_COLOR_RESET "LANG=%s  TZ=%s",env,timezone);
    
    // Get current time
    time(&t);
    tm = localtime(&t);
    // Format the time in 12-hour format with AM/PM
    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", tm);
    printf(DEFAULT_COLOR"Time:\t\t"ANSI_COLOR_RESET "%s\n",buffer);
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
                    printf(ANSI_COLOR_YELLOW " %s\n" ANSI_COLOR_RESET, status);
                } else if (strcmp(status, "Charging") == 0) {
                    printf(ANSI_COLOR_GREEN " %s\n" ANSI_COLOR_RESET, status);
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
    int thread_count = 0,process_count =0;
    struct dirent *entry,*task_entry;
    DIR *dir,*tdir;
    char path[MAX_PATH];

    dir = opendir("/proc");
    if (!dir) return;
    while ((entry = readdir(dir)) != NULL) {
        if (!is_pid_directory(entry->d_name)) continue;
        process_count++;
        snprintf(path,sizeof(path),"/proc/%s/task",entry->d_name);
        tdir = opendir(path);
        if (!dir) continue;
        while ((task_entry =readdir(tdir)) != NULL) {
            if (!is_pid_directory(entry->d_name)) continue;
            thread_count++;
        }
        closedir(tdir);
    }
    closedir(dir);
    printf(DEFAULT_COLOR "Processes:  "ANSI_COLOR_RESET "%d\t"
           DEFAULT_COLOR "Threads:  "ANSI_COLOR_RESET "%d\n",process_count,thread_count);
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

static void systeminfo() {
    distro_name();
    print_hostname_and_kernel();
    print_loaded_modules();
    print_firmware_info();
    print_systemd_info();
    print_cpu_info();
    print_gpu_info();
    if (!is_hypervisor_present()) print_disk_info();  // disk cannot be detected on vms it will fail
    print_memory_and_uptime();
    print_load_average();
    if (!is_hypervisor_present()) print_battery_info(); // also virtual machines don't have a battery
    print_desktop_environment();
    package_manager();
    print_locales_info();
    print_process_and_thread_count();
    print_user_and_group_info();
    printf(ANSI_COLOR_YELLOW "Getting security modules...\n" ANSI_COLOR_RESET);
    LinuxSecurityModule();
}

static void help() {
    printf("-h      print this message and exit\n");
    printf("-p      get supplied process id information\n");
    printf(" -i     specify interval to monitor process (optional)\n\n");
    printf("-H      get hardware information\n");
    printf(" -e     get extended hardware information\n\n");
    printf("-n      get network information\n\n");
    printf("Usage: ./systeminfo -[p|H|n] -[e|i]\n");
}
int main(int argc, char *argv[])
{
    printf(ANSI_COLOR_GREEN "system enumeration\n" ANSI_COLOR_RESET);
    int opt,H_flag = 0,N_flag= 0,P_flag=0,E_flag=0,interv_flag=0;
    int p_value=0,interval=0;
	//Parse command line options
    while ((opt = getopt(argc, argv, "p:i:Hnh")) != -1) {
        switch (opt) {
            case 'p':
                P_flag=1;
                p_value = atoi(optarg);
                break;
            case 'i':
                if (!P_flag) {
                    interval = atoi(optarg);
                }
                interv_flag = 1;
                interval = atoi(optarg);
                break;
            case 'H':
                H_flag= 1;
                if (optind < argc && argv[optind][0] == '-' && argv[optind][1] == 'e' && argv[optind][2] == '\0') {
                E_flag = 1; // Enable `-e`
                optind++;    // Manually consume `-e`
                }
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
                help();
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
        if (!interv_flag) interval = 0;
        getProcessInfo(p_value,interval);
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
    if (E_flag) {
        printf(ANSI_COLOR_YELLOW "Getting pci devices\n"ANSI_COLOR_RESET);
        list_pci_devices();
        printf(ANSI_COLOR_YELLOW "Getting sensor information..\n"ANSI_COLOR_RESET);
        detect_sensors();
        print_battery_information();
    }
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
