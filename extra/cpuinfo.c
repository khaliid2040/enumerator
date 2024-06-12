#include "../main.h"
#include <cpuid.h>
void cpuid() {
    /* i intentionally compared leaf 0 and 1 because this is the only leaf values is being worked 
    it may be removed from the future */
    for (int i=0;i<2;i++) {
        if (i==0) {
            unsigned int eax,ebx,ecx,edx;
            __get_cpuid(i,&eax,&ebx,&ecx,&edx);
            char vendor[13];
            memcpy(vendor, &ebx, 4);
            memcpy(vendor+4, &edx, 4);
            memcpy(vendor+8, &ecx, 4);
            vendor[12] = '\0'; // Null-terminate the string
            printf("CPU vendor: %s\n", vendor);
        } else{
            unsigned int eax, ebx, ecx, edx;
            // Call CPUID instruction to retrieve information
            __get_cpuid(i, &eax, &ebx, &ecx, &edx);
            char *feature_names[]= {"fpu","vme","de","pse","tsc","pae","mce","cx8","apic","VMX"};
            printf("supported features: ");
            for (int j = 0; j < sizeof(feature_names) / sizeof(feature_names[0]); j++) {
                if ((edx >> j) & 1 && (j < 32 ? (ecx >> j) & 1 : (ecx >> (j - 32)) & 1)) {
                    printf("%s ", feature_names[j]);
                }
            }
            printf("\n\n");
        }
    }
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
    printf("Number of cores: %d\n", cores_count /2);
    printf("number of processors: %d\n",processors_count);
    // now getting the vendor 
    cpuid();
    return 0;
}