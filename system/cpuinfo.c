#include "../main.h"
#include <dirent.h>

static unsigned int eax,ebx,ecx,edx;
static char vendor[13];
#if defined(__x86_64__) || defined(__i386__)

static inline bool is_5lvl_supported() {
    __cpuid_count(0x07,0,eax,ebx,ecx,edx);
    if (ecx & (1 << 16))
        return true;
    return false;
}

static void cpuid() {
    /* i intentionally compared leaf 0 and 1 because this is the only leaf values is being worked 
    it may be removed from the future */
    for (int i=0;i<2;i++) {
        if (i==0) {
            __get_cpuid(i,&eax,&ebx,&ecx,&edx);
            memcpy(vendor, &ebx, 4);
            memcpy(vendor+4, &edx, 4);
            memcpy(vendor+8, &ecx, 4);
            vendor[12] = '\0'; // Null-terminate the string
            printf(DEFAULT_COLOR "Vendor:\t\t\t"ANSI_COLOR_RESET "%s\n", vendor);
        } else{
            // Call CPUID instruction to retrieve information
            __get_cpuid(i, &eax, &ebx, &ecx, &edx);
            const char *feature_names[] = {
            "FPU", "VME", "DE", "PSE", "TSC", "PAE", "MCE", "CX8",
            "APIC", "SEP", "MTRR", "PGE", "MCA", "CMOV", "PAT", "PSE-36",
            "MMX", "FXSR", "SSE", "SSE2", "SS"};
            // Check for features in EDX
            printf(DEFAULT_COLOR "Supported features:\t"ANSI_COLOR_RESET);
            for (int j = 0; j < 32; j++) {
                if ((edx >> j) & 1) {
                    if (j < (sizeof(feature_names) / sizeof(feature_names[0]))) {
                        printf("%s ", feature_names[j]);
                    }
                }
            }
            // 5 level paging
            if (is_5lvl_supported()) printf("5lvl ");
            //array of integers of correspond to simd instruction
            const int simd[]= {0,19,20};
            const char *simd_features[  ]= {"SSE3","SSE4.1","SSE4.2"};
            for (int j=0; j<3; j++) {
                if (ecx & ( 1 << simd[j])) {
                    printf("%s  ", simd_features[j]); 
                }
            }
                // now for hyperthreading and simulatanous threading
            if (ecx & (1<<28)) {
                if (strcmp(vendor,"GenuineIntel")==0) {
                    printf("HT ");
                } else if (strcmp(vendor,"AuthenticAMD")==0) {
                    printf("SMT ");
                    }
                } 
            Virtualization virt = detect_hypervisor();
            switch (virt) {
                case KVM: printf("KVM"); break;
                case Virtualbox: printf("Virtualbox"); break;
                case Vmware: printf("Vmware"); break;
                case hyperv: printf("Microsoft hyper-v"); break;
                case xen: printf("Xen"); break;
                case unknown: printf(ANSI_COLOR_RED"unknown"ANSI_COLOR_RESET); break;
            }
        }
        
        }
}
#endif
/**
 * This code entirely works well on x86 architecture but for other architecture it maybe used
 * a more generic way like parsing /proc or /sys.
 * On arm this below small code should be enough for now
 */
#if defined(__arm__) || defined(__aarch64__)
uint64_t read_midr_el1() {
    uint64_t val;
    asm volatile("mrs %0, MIDR_EL1" : "=r"(val));
    return val;
}

const char* identify_model(uint16_t part) {
    switch (part) {
        case 0xD03: return "Cortex-A53";
        case 0xD04: return "Cortex-A35";
        case 0xD05: return "Cortex-A55";
        case 0xD07: return "Cortex-A57";
        case 0xD08: return "Cortex-A72";
        case 0xD09: return "Cortex-A73";
        case 0xD0A: return "Cortex-A75";
        case 0xD0B: return "Cortex-A76";
        case 0xD41: return "Cortex-A78";
        case 0xD42: return "Cortex-X1";
        case 0xD44: return "Cortex-A710";
        case 0xD46: return "Cortex-X2";
        case 0xD47: return "Cortex-A715";
        case 0xD48: return "Cortex-X3";
        case 0xD4A: return "Cortex-A720";
        case 0xD4B: return "Cortex-X4";
        default:    return "Unknown";
    }
}

const char* identify_vendor(uint8_t impl) {
    switch (impl) {
        case 0x41: return "ARM Ltd";
        case 0x42: return "Broadcom";
        case 0x43: return "Cavium";
        case 0x44: return "DEC";
        case 0x4E: return "NVIDIA";
        case 0x50: return "AppliedMicro";
        case 0x51: return "Qualcomm";
        case 0x56: return "Marvell";
        case 0x61: return "Apple";
        default:   return "Unknown";
    }
}

static void generic_cpuinfo() {
    uint64_t midr = read_midr_el1();
    uint8_t implementer = (midr >> 24) & 0xFF;
    uint8_t variant     = (midr >> 20) & 0xF;
    uint8_t architecture = (midr >> 16) & 0xF;
    uint16_t part       = (midr >> 4) & 0xFFF;
    uint8_t revision    = midr & 0xF;

    printf(DEFAULT_COLOR "Processor:\t\t" ANSI_COLOR_RESET "%s\n",identify_vendor(implementer));
    printf(DEFAULT_COLOR "Model:\t\t" ANSI_COLOR_RESET "%s\n",identify_model(part));
    printf(DEFAULT_COLOR "Revision:\t\t" ANSI_COLOR_RESET "r%dp%d\n", variant, revision);
    printf(DEFAULT_COLOR "Architecture:\t\t" ANSI_COLOR_RESET "0x%X\n", architecture);

}
#endif      
static int cpu_vulnerabilities(void) {
    struct dirent *entry;
    char *dir_path= "/sys/devices/system/cpu/vulnerabilities";
    char file_path[MAX_LINE_LENGTH];
    DIR *path= opendir(dir_path);
    if (path ==NULL) {
        perror("couldn't open");
        return 2;
    }
    while ((entry= readdir(path)) != NULL) {
        if (entry->d_type == DT_REG) {
           // printf("file %s\n",entry->d_name);
            snprintf(file_path,sizeof(file_path),"%s/%s",dir_path,entry->d_name);
            process_file(file_path,entry->d_name);
        }
    }
    closedir(path);
}
static struct freq frequency(void) {
    struct freq frq = {0};
    // Initialize frequency values to some default or invalid value
    frq.max_freq = 0;
    frq.min_freq = 0;
    frq.base_freq = 0;

    DIR *policies = opendir("/sys/devices/system/cpu/cpufreq");
    if (policies == NULL) {
        perror("opendir");
        return frq;
    }

    char path[256], contents[20];
    struct dirent *entry;

    while ((entry = readdir(policies)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Process each directory entry (policy)
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/scaling_max_freq", entry->d_name);
        FILE *maxfp = fopen(path, "r");
        if (maxfp != NULL) {
            if (fgets(contents, sizeof(contents), maxfp) != NULL) {
                frq.max_freq = strtoul(contents, NULL, 0);
            }
            fclose(maxfp);
        }

        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/scaling_min_freq", entry->d_name);
        FILE *minfp = fopen(path, "r");
        if (minfp != NULL) {
            if (fgets(contents, sizeof(contents), minfp) != NULL) {
                frq.min_freq = strtoul(contents, NULL, 0);
            }
            fclose(minfp);
        }

        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/base_frequency", entry->d_name);
        FILE *basefp = fopen(path, "r");
        if (basefp != NULL) {
            if (fgets(contents, sizeof(contents), basefp) != NULL) {
                frq.base_freq = strtoul(contents, NULL, 0);
            }
            fclose(basefp);
        }
    }

    closedir(policies);
    return frq;
}

static void hwmon() {
    printf("\n");
    struct dirent *entry;
    char path[SIZE],contents[SIZE];
    bool set=false;
    DIR *hw= opendir("/sys/class/hwmon/");
    if (hw==NULL) {
        perror("dir");
        return;
    }

    while ((entry=readdir(hw)) != NULL) {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) {
            continue;
        }
        snprintf(path,SIZE,"/sys/class/hwmon/%s/name",entry->d_name);
        FILE *temfp= fopen(path,"r");
        if (temfp==NULL) {
            perror("fopen");
            continue;
        }
        if (fgets(contents,SIZE,temfp) != NULL) {
            contents[strcspn(contents, "\n")] = '\0';   
            if (!strcmp(contents,"coretemp")) {
                set=true;
            }
        }
        fclose(temfp);  
        if (set) {
            for (int i=1; ; i++) {
                snprintf(path,SIZE,"/sys/class/hwmon/%s/temp%d_input",entry->d_name,i);
                FILE *tem= fopen(path,"r")  ;
                if (tem==NULL) {        
                    break;
                }
                float cur_temp;
                if (fgets(contents,SIZE,tem) != NULL) {
                    /*if (i==1) {
                        printf(DEFAULT_COLOR "Package:\t\t" ANSI_COLOR_RESET);
                    } else {
                        printf(DEFAULT_COLOR "Core %d:\t\t\t" ANSI_COLOR_RESET,i - 2);
                    }*/
                   cur_temp = strtof(contents,NULL) / 1000.0;
                   snprintf(path,sizeof(path),"/sys/class/hwmon/%s/temp%d_label",entry->d_name,i);
                   FILE *label = fopen(path,"r");
                   if (label) {
                    if (fgets(contents,SIZE,label) != NULL) {
                        int len = strlen(contents);
                        // remove new line character
                        if (contents[len -1] == '\n') {
                            contents[len - 1] = '\0';
                        }
                        printf(DEFAULT_COLOR "%-10s:\t\t%.1f °C "ANSI_COLOR_RESET,contents,cur_temp);
                    }
                    fclose(label);
                   } 
                }
                fclose(tem);    
                snprintf(path,SIZE,"/sys/class/hwmon/%s/temp%d_crit",entry->d_name,i);
                FILE *level = fopen(path,"r");
                if (level==NULL) {  
                    break;
                } 
                if (fgets(contents,SIZE,level) !=NULL) {
                    float crit_level = strtof(contents,NULL) / 1000.0;
                    if ( cur_temp >= crit_level) {
                        printf(ANSI_COLOR_RED "Critical\n"ANSI_COLOR_RESET);
                    } else if (cur_temp < crit_level) {
                        printf(ANSI_COLOR_GREEN "Normal\n"ANSI_COLOR_RESET);
                    }
                }
                fclose(level);
            }
            set=false;  
        }
        }
    
    closedir(hw);
}
static bool get_arch_and_endianess(char* arch,char*endianess,size_t len) {
    FILE *fp;

    fp = fopen("/sys/kernel/cpu_byteorder","r");
    if (!fp)
        return false;
    if (fgets(endianess,len,fp) == NULL) {
        fclose(fp);
        return false;
    }
    fclose(fp);
    fp = fopen("/sys/kernel/address_bits","r");
    if (!fp)
        return false;
    if (fgets(arch,len,fp) == NULL) {
        fclose(fp);
        return false;
    }
    //remove /n
    arch[strlen(arch) - 1] = '\0';
    endianess[strlen(endianess) - 1] = '\0';
    fclose(fp);
    return true;
}
/*This code is wierd so let me explain:
  * first in order to get number cpu sockets we gonna use the last successful read of 
  * /sys/devices/system/cpu/cpu[N]/topology/physical_package_id the last one which correspond 
  * to last thread of all threads across sockets. So the math will be last physical id + 1 so
  * effectively translate to from 0-based count to 1-based count
  * on failure the function will return 0, and on success will return the number of sockets*/
static int get_cpu_sockets() {
    int sockets =-1; // initially one
    FILE *fp;
    char buffer[5],path[72];
    unsigned int count=0;
    while (1) {
    snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpu%d/topology/physical_package_id",count);
    count++;
    fp = fopen(path,"r");
    if (!fp)
        break;
    if (fgets(buffer,sizeof(buffer),fp) == NULL) {
        fclose(fp);
        break;
    }
    fclose(fp);
    }
    sockets = atoi(buffer);
    return sockets +1;
}

static void print_cache_info(int cores) {
    unsigned int cache_count = 0;
    unsigned int cache_type, cache_level, cache_size;
    unsigned int ways, partitions, line_size, sets;
    unsigned int cache_sharing;
    char unit[4];

    while (1) {
        #if defined(__x86_64) || defined(__i386__)
        __cpuid_count(0x4, cache_count, eax, ebx, ecx, edx);
        #endif
        cache_type = eax & 0x1F;
        if (cache_type == 0) {
            break;
        }
        cache_level = (eax >> 5) & 0x7;       // Bits 5-7: Cache level (L1, L2, L3, etc.)
        ways = ((ebx >> 22) & 0x3FF) + 1;
        partitions = ((ebx >> 12) & 0x3FF) + 1;
        line_size = (ebx & 0xFFF) + 1;
        sets = ecx + 1;
        cache_size = ways * partitions * line_size * sets;
        cache_size /= 1024;
        cache_sharing = ((eax >> 14) & 0xfff);
        if (cache_sharing == 0) cache_sharing = cores;
        if (cache_sharing == 0) cache_sharing = 1;
        unsigned int instance = cores / cache_sharing;
        if (instance == 0) instance = 1;

        double converted_size = convert_size_unit((double)cache_size * instance, unit, sizeof(unit));

        printf(DEFAULT_COLOR "\nCache L%d:\t\t" ANSI_COLOR_RESET "type: %s\n\t\t\tsize: %.1f %s\n\t\t\tinstances: %d\n\t\t\tAssociativity: %d-ways\n", cache_level,
                cache_type == 1 ? "Data cache" :
                cache_type == 2 ? "Instruction cache" :
                cache_type == 3 ? "Unified cache" : "Unknown",
                converted_size, unit, instance, ways);

        cache_count++;
    }
}

void cpuinfo() {
    printf(ANSI_COLOR_YELLOW "getting processor information\n" ANSI_COLOR_RESET);

    unsigned int sockets = get_cpu_sockets(); 
    int cores = 0, processors = 0;
    if (count_processor(&cores, &processors)) {
        printf(DEFAULT_COLOR "cores:\t\t\t" ANSI_COLOR_RESET "%d\n", cores);
        printf(DEFAULT_COLOR "processor:\t\t" ANSI_COLOR_RESET "%d\n", processors);
    }
    printf(DEFAULT_COLOR "Sockets:\t\t" ANSI_COLOR_RESET "%d\n", sockets);

    if (!is_hypervisor_present()) {
        struct freq frq = frequency();
        float max = frq.max_freq / 1e6; // 1e6 = 1000000.0
        float min = frq.min_freq / 1e3; // 1e3 = 1000   
        float base = frq.base_freq / 1e6; // 1e6 = 1000000.0
        printf(DEFAULT_COLOR "Frequency:\t\t" ANSI_COLOR_RESET "max: %.1f GHz  min: %.1f MHz  base: %.1f GHz\n", max, min, base);
    }

    #ifdef supported    
    cpuid();
    char brand[50];
    for (int i = 0; i < 3; ++i) {
        __get_cpuid(0x80000002 + i, &eax, &ebx, &ecx, &edx);
        memcpy(brand + i * 16, &eax, 4);
        memcpy(brand + i * 16 + 4, &ebx, 4);
        memcpy(brand + i * 16 + 8, &ecx, 4);
        memcpy(brand + i * 16 + 12, &edx, 4);
    }
    brand[48] = '\0';
    printf(DEFAULT_COLOR "\nBrand:\t\t\t" ANSI_COLOR_RESET "%s\n", brand);

    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    unsigned int family = ((eax >> 8) & 0x0F) + ((eax >> 20) & 0xFF);
    unsigned int model = ((eax >> 4) & 0x0F) + ((eax >> 12) & 0xF0);
    unsigned int stepping = eax & 0x0F;
    printf(DEFAULT_COLOR "Family\t\t\t" ANSI_COLOR_RESET "%u\n", family);
    printf(DEFAULT_COLOR "Model\t\t\t" ANSI_COLOR_RESET "%u\n", model);
    printf(DEFAULT_COLOR "Stepping\t\t" ANSI_COLOR_RESET "%u\n", stepping);
    #else
    generic_cpuinfo();
    #endif

    char arch[10], endianess[10];
    if (get_arch_and_endianess(arch, endianess, 10)) {
        printf(DEFAULT_COLOR "Architecture:\t\t" ANSI_COLOR_RESET "%s-bit\n", arch);
        printf(DEFAULT_COLOR "Endianess:\t\t" ANSI_COLOR_RESET "%s", endianess);
    }

    hwmon();
    print_cache_info(cores);
    printf(ANSI_COLOR_BLUE "CPU Vulnerabilities:\n" ANSI_COLOR_RESET);
    cpu_vulnerabilities();
}

