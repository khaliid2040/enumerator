#include "../main.h"
#include <dirent.h>
unsigned int eax,ebx,ecx,edx;
char vendor[13];
void cpuid() {
    /* i intentionally compared leaf 0 and 1 because this is the only leaf values is being worked 
    it may be removed from the future */
    for (int i=0;i<2;i++) {
        if (i==0) {
            __get_cpuid(i,&eax,&ebx,&ecx,&edx);
            memcpy(vendor, &ebx, 4);
            memcpy(vendor+4, &edx, 4);
            memcpy(vendor+8, &ecx, 4);
            vendor[12] = '\0'; // Null-terminate the string
            printf(ANSI_COLOR_LIGHT_GREEN "Vendor:\t\t\t"ANSI_COLOR_RESET "%s\n", vendor);
        } else{
            // Call CPUID instruction to retrieve information
            __get_cpuid(i, &eax, &ebx, &ecx, &edx);
            const char *feature_names[] = {
            "FPU", "VME", "DE", "PSE", "TSC", "PAE", "MCE", "CX8",
            "APIC", "SEP", "MTRR", "PGE", "MCA", "CMOV", "PAT", "PSE-36",
            "MMX", "FXSR", "SSE", "SSE2", "SS"};
            // Check for features in EDX
            printf(ANSI_COLOR_LIGHT_GREEN "Supported features:\t"ANSI_COLOR_RESET);
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
            //detecting the hypervisor in case if we run on virtual machine
            if (ecx & (1 << 31)) {
                printf(ANSI_COLOR_LIGHT_GREEN "\nVirtualization detected: "ANSI_COLOR_RESET);
                char *hypervisors[] = {"KVM", "Vmware", "Virtualbox", "hyper-v"};
                int sig[3];
                // Execute CPUID instruction to retrieve hypervisor signature
                __cpuid(0x40000000U, eax, ebx, ecx, edx);

                // Store the retrieved signature
                sig[0] = ebx;
                sig[1] = ecx;
                sig[2] = edx;
                // For demonstration purposes, let's print the signature
                if (sig[0]==0x4B4D564B && sig[1]==0x564B4D56 && sig[2]== 0x0000004D) { 
                    printf("KVM");
                } else if (sig[0]==0x61774D56 && sig[1]==0x4D566572 && sig[3]==0x65726175) {
                    printf("VMWare");
                }else if (sig[0]==0x72754D56 && sig[1]==0x56656361 && sig[2]==0x32746E65) {
                    printf("Virtualbox");
                }else if (sig[0]==0x72636968 && sig[1]==0x4D566572 && sig[2]==0x65746E65) {
                    printf("Hyper-v");
                }else {
                    printf("unknown");
                }
            }
        }
        
        }
}
//the function above is good and optimal uses direct cpuid instrcution but it only available on x86/x86_64 so 
//for other architectures we don't have option but to parse /proc/cpuinfo

void generic_cpuinfo(struct Cpuinfo *cpu) {
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
int cpu_vulnerabilities(void) {
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
struct freq *frequency(void) {
    struct freq *frq = malloc(sizeof(struct freq));
    if (frq == NULL) {
        perror("malloc");
        return NULL;
    }

    // Initialize frequency values to some default or invalid value
    frq->max_freq = 0;
    frq->min_freq = 0;
    frq->base_freq = 0;

    DIR *policies = opendir("/sys/devices/system/cpu/cpufreq");
    if (policies == NULL) {
        perror("opendir");
        free(frq);
        return NULL;
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
                frq->max_freq = strtoul(contents, NULL, 0);
            }
            fclose(maxfp);
        }

        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/scaling_min_freq", entry->d_name);
        FILE *minfp = fopen(path, "r");
        if (minfp != NULL) {
            if (fgets(contents, sizeof(contents), minfp) != NULL) {
                frq->min_freq = strtoul(contents, NULL, 0);
            }
            fclose(minfp);
        }

        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpufreq/%s/base_frequency", entry->d_name);
        FILE *basefp = fopen(path, "r");
        if (basefp != NULL) {
            if (fgets(contents, sizeof(contents), basefp) != NULL) {
                frq->base_freq = strtoul(contents, NULL, 0);
            }
            fclose(basefp);
        }
    }

    closedir(policies);
    return frq;
}

void hwmon() {
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
                    if (i==1) {
                        printf(ANSI_COLOR_LIGHT_GREEN "Package:\t\t" ANSI_COLOR_RESET);
                    } else {
                        printf(ANSI_COLOR_LIGHT_GREEN "Core %d:\t\t\t" ANSI_COLOR_RESET,i - 2);
                    }
                    cur_temp = strtof(contents,NULL) / 1000.0;
                    printf("%.1f °C   ",cur_temp);
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
int cpuinfo() {
        printf(ANSI_COLOR_YELLOW "getting processor information\n" ANSI_COLOR_RESET);
    
    /*
        cpuinfo_buffer holds the buffer of the cpuinfo file
        buffer_size is the size of the buffer
        processors and cores are strings searched in  the file
        
    */      
    int cores=0,processors=0,level=4; //assumption: 4 cache levels
    char spath[60],tpath[60];
    char size_cont[20],type_cont[30];
    if (count_processor(&cores,&processors)) {
        printf(ANSI_COLOR_LIGHT_GREEN "cores:\t\t\t"ANSI_COLOR_RESET "%d\n",cores /2);
        printf(ANSI_COLOR_LIGHT_GREEN "processor:\t\t" ANSI_COLOR_RESET "%d\n",processors);
    }
    //frequency got via sysfs as 
    struct freq *frq= frequency();
    if (frq == NULL) {
        perror("malloc");
        return -1;
    }
    float max= frq->max_freq / 1e6; // 1e6 = 1000000.0
    float min = frq->min_freq / 1e3; // 1e3= 1000   
    float base = frq->base_freq / 1e6; // 1e6 = 1000000.0
    printf(ANSI_COLOR_LIGHT_GREEN "Frequency:\t\t"ANSI_COLOR_RESET "max: %.1f GHz  min: %.1f MHz  base: %.1f GHz\n\n", max,min,base);
    free(frq);
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
    printf(ANSI_COLOR_LIGHT_GREEN "\nBrand:\t\t\t"ANSI_COLOR_RESET  "%s\n", brand);
    // cpu family stepping and model
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    
    unsigned int family = ((eax >> 8) & 0x0F) + ((eax >> 20) & 0xFF);
    unsigned int model = ((eax >> 4) & 0x0F) + ((eax >> 12) & 0xF0);
    unsigned int stepping = eax & 0x0F;
    printf(ANSI_COLOR_LIGHT_GREEN"Family\t\t\t"ANSI_COLOR_RESET "%u\n",family);
    printf(ANSI_COLOR_LIGHT_GREEN"Model\t\t\t"ANSI_COLOR_RESET "%u\n",model);
    printf(ANSI_COLOR_LIGHT_GREEN"Stepping\t\t"ANSI_COLOR_RESET "%u\n",stepping);
    #else
        struct Cpuinfo cpu;
        generic_cpuinfo(&cpu);
        printf(ANSI_COLOR_LIGHT_GREEN "\nVendor:\t\t\t"ANSI_COLOR_RESET  "%s", cpu.vendor);
        printf(ANSI_COLOR_LIGHT_GREEN "\nBrand:\t\t\t"ANSI_COLOR_RESET  "%s\n", cpu.model_name);
        printf(ANSI_COLOR_LIGHT_GREEN"Family\t\t\t"ANSI_COLOR_RESET "%u\n",cpu.family);
        printf(ANSI_COLOR_LIGHT_GREEN"Model\t\t\t"ANSI_COLOR_RESET "%u\n",cpu.model);
        printf(ANSI_COLOR_LIGHT_GREEN"Stepping\t\t"ANSI_COLOR_RESET "%u\n",cpu.stepping);
    #endif
    hwmon();
    for (int i=0;i<processors;i++) {
        level--; //decrement each iteration and if fopen fails assume non exsted
        if (level <0) {
            break;  // prevent level becoming negative  
        }
        snprintf(spath,sizeof(spath),"/sys/devices/system/cpu/cpu%d/cache/index%d/size",i,level); //size
        snprintf(tpath,sizeof(tpath),"/sys/devices/system/cpu/cpu%d/cache/index%d/type",i,level);
        FILE *sizeN= fopen(spath,"r");
        if (sizeN==NULL) {
            continue;   
        }
        if (fgets(size_cont,sizeof(size_cont),sizeN) == NULL) {
            perror("fgets");
            fclose(sizeN);
            continue;
        }   
        fclose(sizeN);
        //now for cache type
        FILE *typeN= fopen(tpath,"r");
        if (typeN==NULL) {
            perror("fopen");
            return 2;
        }
        if (fgets(type_cont,sizeof(type_cont),typeN) == NULL) { 
            perror("fgets");
            fclose(typeN);
            continue;
        }
        fclose(typeN);
        printf(ANSI_COLOR_LIGHT_GREEN "L%d Cache :"ANSI_COLOR_RESET "\t\t type: %s\t\t\t size: %s\n",level,type_cont,size_cont);
    }
    printf(ANSI_COLOR_YELLOW "Getting cpu vurnuabilities\n" ANSI_COLOR_RESET);
    cpu_vulnerabilities();
    return 0;
}