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
            printf(ANSI_COLOR_LIGHT_GREEN "Vendor:\t\t\t\t\t"ANSI_COLOR_RESET "%s\n", vendor);
        } else{
            // Call CPUID instruction to retrieve information
            __get_cpuid(i, &eax, &ebx, &ecx, &edx);
            char *feature_names[]= {"fpu","vme","de","pse","tsc","pae","mce","cx8","apic","VMX"};
            printf(ANSI_COLOR_LIGHT_GREEN "supported features:\t\t\t"ANSI_COLOR_RESET);
            for (int j = 0; j < sizeof(feature_names) / sizeof(feature_names[0]); j++) {
                if ((edx >> j) & 1 && (j < 32 ? (ecx >> j) & 1 : (ecx >> (j - 32)) & 1)) {
                    printf("%s ", feature_names[j]);
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
                printf(ANSI_COLOR_LIGHT_GREEN "\nVirtualization detected:\t\t "ANSI_COLOR_RESET);
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
    struct freq *frq= malloc(sizeof(struct freq));
    if (frq==NULL) {
        perror("malloc");
        return NULL;
    }
    DIR *policies= opendir("/sys/devices/system/cpu/cpufreq");
    int count_entries=0;
    char path[256],contents[20];
    struct dirent *entry;
    while ((entry=readdir(policies)) != NULL) {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) {
            continue;
        }

        snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpufreq/%s/scaling_max_freq",entry->d_name);
        FILE *maxfp= fopen(path,"r");
        if (maxfp==NULL) {
            perror("fopen");
            continue;
        }
        if (fgets(contents,sizeof(contents),maxfp) != NULL) {
            unsigned long max_freq =strtoul(contents,NULL,0);
            frq->max_freq=max_freq;

        }
        fclose(maxfp);
        snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpufreq/%s/scaling_min_freq",entry->d_name);
        FILE *base= fopen(path,"r");
        if (base==NULL) {
            perror("fopen");
            continue;
        }
        if (fgets(contents,sizeof(contents),base) !=NULL) {
            unsigned long min_freq= strtoul(contents,NULL,0);
            frq->min_freq=min_freq;
        }
        fclose(base);
        snprintf(path,sizeof(path),"/sys/devices/system/cpu/cpufreq/%s/base_frequency",entry->d_name);
        FILE *minfp= fopen(path,"r");
        if (minfp==NULL) {
            perror("fopen");
            continue;
        }
        if (fgets(contents,sizeof(contents),minfp) !=NULL) {
            unsigned long base_freq= strtoul(contents,NULL,0);
            frq->base_freq=base_freq;
        }
        fclose(minfp);
    }
    closedir(policies);
    return frq;
}
void hwmon() {
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
                        printf(ANSI_COLOR_LIGHT_GREEN "Package:  " ANSI_COLOR_RESET);
                    } else {
                        printf(ANSI_COLOR_LIGHT_GREEN "Core %d:  " ANSI_COLOR_RESET,i - 2);
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
        printf(ANSI_COLOR_LIGHT_GREEN "cores:\t\t"ANSI_COLOR_RESET "%d\n",cores /2);
        printf(ANSI_COLOR_LIGHT_GREEN "processor:\t" ANSI_COLOR_RESET "%d\n",processors);
    }
    //frequency got via sysfs as 
    struct freq *frq= frequency();
    if (frq == NULL) {
        perror("malloc");
        return -1;
    }
    float max= frq->max_freq / 1000000.0;
    float min = frq->min_freq / 100000.0;
    float base = frq->base_freq / 1000000.0;
    printf(ANSI_COLOR_LIGHT_GREEN "Frequency:\t"ANSI_COLOR_RESET "max: %.1f GHz  min: %.1f GHz  base: %.1f GHz\n\n", max,min,base);
    free(frq);
    //temperature
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
        printf("L%d:\t type: %s\t size: %s\n",level,type_cont,size_cont);
    }
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
    printf(ANSI_COLOR_LIGHT_GREEN "\nBrand:\t\t\t\t\t"ANSI_COLOR_RESET  "%s\n", brand);
    printf(ANSI_COLOR_YELLOW "Getting cpu vurnuabilities\n" ANSI_COLOR_RESET);
    #endif  
    cpu_vulnerabilities();
    return 0;
}