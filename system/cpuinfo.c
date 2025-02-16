#include "../main.h"
#include <dirent.h>

unsigned int eax,ebx,ecx,edx;
char vendor[13];
#ifdef supported

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
            for (int i = 0; i < 32; i++) {
                if ((edx >> i) & 1) {
                    if (i < (sizeof(feature_names) / sizeof(feature_names[0]))) {
                        printf("%s ", feature_names[i]);
                    }
                }
            }
            //array of integers of correspond to simd instruction
            const int simd[]= {0,19,20};
            const char *simd_features[  ]= {"SSE3","SSE4.1","SSE4.2"};
            for (int i=0; i<3; i++) {
                if (ecx & ( 1 << simd[i])) {
                    printf("%s  ", simd_features[i]); 
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
//the function above is good and optimal uses direct cpuid instrcution but it only available on x86/x86_64 so 
//for other architectures we don't have option but to parse /proc/cpuinfo
#ifndef supported
static void generic_cpuinfo(struct Cpuinfo *cpu) {
    char buffer[256];  // Increase buffer size to handle longer lines
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        perror("fopen");
        return;
    }

    // Initialize CPU info to default values
    cpu->vendor[0] = '\0';
    cpu->model = 0;
    cpu->family = 0;
    cpu->stepping = 0;
    cpu->model_name[0] = '\0';

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Check the start of each line and parse accordingly
        if (strncmp(buffer, "vendor_id", 9) == 0) {
            sscanf(buffer, "vendor_id%*[ \t:]    %s", cpu->vendor);
        } else if (strncmp(buffer, "cpu family", 10) == 0) {
            sscanf(buffer, "cpu family%*[ \t:]    %u", &cpu->family);
        } else if (strncmp(buffer, "model", 5) == 0 && !strstr(buffer, "model name")) {
            sscanf(buffer, "model%*[ \t:]%u", &cpu->model);
        } else if (strncmp(buffer, "model name", 10) == 0) {
            sscanf(buffer, "model name%*[ \t:]%[^\n]", cpu->model_name);
        } else if (strncmp(buffer, "stepping", 8) == 0) {
            sscanf(buffer, "stepping%*[ \t:]%d", &cpu->stepping);
        }
    }
    
    fclose(fp);
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
                   }
                    fclose(label);
                    //printf("%.1f °C  ",cur_temp);
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
int cpuinfo() {
        printf(ANSI_COLOR_YELLOW "getting processor information\n" ANSI_COLOR_RESET);
    
    /*
        cpuinfo_buffer holds the buffer of the cpuinfo file
        buffer_size is the size of the buffer
        processors and cores are strings searched in  the file
        
    */     
    unsigned int sockets = get_cpu_sockets(); 
    int cores=0,processors=0,level=4; //assumption: 4 cache levels
    char spath[60],tpath[60];
    char size_cont[20],type_cont[30];
    if (count_processor(&cores,&processors)) {
        printf(DEFAULT_COLOR "cores:\t\t\t"ANSI_COLOR_RESET "%d\n",cores);
        printf(DEFAULT_COLOR "processor:\t\t" ANSI_COLOR_RESET "%d\n",processors);
    }
   
    printf(DEFAULT_COLOR"Sockets:\t\t"ANSI_COLOR_RESET "%d\n",sockets);
    //frequency got via sysfs as 
    struct freq frq= frequency();
    float max= frq.max_freq / 1e6; // 1e6 = 1000000.0
    float min = frq.min_freq / 1e3; // 1e3= 1000   
    float base = frq.base_freq / 1e6; // 1e6 = 1000000.0
    printf(DEFAULT_COLOR "Frequency:\t\t"ANSI_COLOR_RESET "max: %.1f GHz  min: %.1f MHz  base: %.1f GHz\n\n", max,min,base);
    //temperature
    #ifdef supported    
    // now getting the vendor 
    cpuid();
    //now we are going to print the brand using cpuid instruction
    
    char brand[50];
    for (int i = 0; i < 3; ++i) {
        __get_cpuid(0x80000002 + i, &eax, &ebx, &ecx, &edx);
        memcpy(brand + i * 16, &eax, 4);
        memcpy(brand + i * 16 + 4, &ebx, 4);
        memcpy(brand + i * 16 + 8, &ecx, 4);
        memcpy(brand + i * 16 + 12, &edx, 4);
    }
    brand[48] = '\0';
    printf(DEFAULT_COLOR "\nBrand:\t\t\t"ANSI_COLOR_RESET  "%s\n", brand);
    // cpu family stepping and model
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    
    unsigned int family = ((eax >> 8) & 0x0F) + ((eax >> 20) & 0xFF);
    unsigned int model = ((eax >> 4) & 0x0F) + ((eax >> 12) & 0xF0);
    unsigned int stepping = eax & 0x0F;
    printf(DEFAULT_COLOR"Family\t\t\t"ANSI_COLOR_RESET "%u\n",family);
    printf(DEFAULT_COLOR"Model\t\t\t"ANSI_COLOR_RESET "%u\n",model);
    printf(DEFAULT_COLOR"Stepping\t\t"ANSI_COLOR_RESET "%u\n",stepping);
    #else
        struct Cpuinfo cpu;
        generic_cpuinfo(&cpu);
        printf(DEFAULT_COLOR "\nVendor:\t\t\t"ANSI_COLOR_RESET  "%s", cpu.vendor);
        printf(DEFAULT_COLOR "\nBrand:\t\t\t"ANSI_COLOR_RESET  "%s\n", cpu.model_name);
        printf(DEFAULT_COLOR"Family\t\t\t"ANSI_COLOR_RESET "%u\n",cpu.family);
        printf(DEFAULT_COLOR"Model\t\t\t"ANSI_COLOR_RESET "%u\n",cpu.model);
        printf(DEFAULT_COLOR"Stepping\t\t"ANSI_COLOR_RESET "%u\n",cpu.stepping);
    #endif
    char arch[10],endianess[10];
    if (get_arch_and_endianess(arch,endianess,10)) {
        printf(DEFAULT_COLOR "Architecture:\t\t"ANSI_COLOR_RESET "%s-bit\n",arch);
        printf(DEFAULT_COLOR "Endianess:\t\t" ANSI_COLOR_RESET "%s",endianess);
    }
    hwmon();
    unsigned int eax, ebx, ecx, edx;
    unsigned int cache_count = 0;
    unsigned int cache_type, cache_level, cache_size;
    unsigned int ways, partitions, line_size, sets;
    unsigned int cache_sharing,associativity;
    char unit[4];


    while (1) {
        #if defined(__x86_64) || defined(__i386__) // at least do not do anything on other architectures
        __cpuid_count(0x4, cache_count, eax, ebx, ecx, edx);
        #endif
        cache_type = eax & 0x1F; // Bits 0-4: Cache type
        if (cache_type == 0) {
            // Cache type 0 means no more caches
            break;
        }
        cache_level = (eax >> 5) & 0x7;       // Bits 5-7: Cache level (L1, L2, L3, etc.)
        ways = ((ebx >> 22) & 0x3FF) + 1;
        partitions = ((ebx >> 12) & 0x3FF) + 1;
        line_size = (ebx & 0xFFF) + 1;
        sets = ecx + 1;
        cache_size = ways * partitions * line_size * sets;
        cache_size /= 1024; // convert size to KiB because convert_unit_size expects size in KiB
        cache_sharing = ((eax >> 14) & 0xfff);
        //associativity = ((ebx >> 22) & 0x3FF) +1; // Extract bits 31:22
        if (cache_sharing == 0) cache_sharing = cores; //if it is zero then they are not sharing
        if (cache_sharing == 0) cache_sharing = 1; // finally if still zero because cores are zero the last resort is assume 1
        unsigned int instance = cores / cache_sharing;
        if (instance == 0) instance = 1; // If instance is zero, then there is only one instance

        double converted_size = convert_size_unit((double)cache_size * instance, unit, sizeof(unit));

        printf(DEFAULT_COLOR"\nCache L%d:\t\t"ANSI_COLOR_RESET "type: %s\n\t\t\tsize: %.1f %s\n\t\t\tinstances: %d\n\t\t\tAssociativity: %d-ways\n", cache_level,
                cache_type == 1 ? "Data cache" :
                cache_type == 2 ? "Instruction cache" :
                cache_type == 3 ? "Unified cache" : "Unknown",
                converted_size, unit,instance, ways);

        cache_count++; // Move to the next cache
    }
    printf(ANSI_COLOR_YELLOW "Getting cpu vurnuabilities\n" ANSI_COLOR_RESET);
    cpu_vulnerabilities();
    return 0;
}
