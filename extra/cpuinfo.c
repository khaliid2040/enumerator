#include "../main.h"
#include <dirent.h>
#if defined(__x86_64__) || defined(__i386__)    
#include <cpuid.h>          
#define supported
#endif
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
            print   f("CPU vendor: %s\n", vendor);
        } else{
            // Call CPUID instruction to retrieve information
            __get_cpuid(i, &eax, &ebx, &ecx, &edx);
            char *feature_names[]= {"fpu","vme","de","pse","tsc","pae","mce","cx8","apic","VMX"};
            printf("supported features: ");
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
                printf("\nVirtualization detected: ");
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
                    printf("KVM\n");
                } else if (sig[0]==0x61774D56 && sig[1]==0x4D566572 && sig[3]==0x65726175) {
                    printf("VMWare\n");
                }else if (sig[0]==0x72754D56 && sig[1]==0x56656361 && sig[2]==0x32746E65) {
                    printf("Virtualbox\n");
                }else if (sig[0]==0x72636968 && sig[1]==0x4D566572 && sig[2]==0x65746E65) {
                    printf("Hyper-v\n");
                }else {
                    printf("unknown\n");
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

int cpuinfo() {
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
    printf("cores: %d\n", cores_count /2);
    printf("processors: %d\n",processors_count);
    fclose(cpuinfo);
    free(cpuinfo_buffer);
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
    printf("\nBrand:  %s\n", brand);
    printf(ANSI_COLOR_YELLOW "Getting cpu vurnuabilities\n" ANSI_COLOR_RESET);
    #endif  
    cpu_vulnerabilities();
    return 0;
}