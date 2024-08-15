#include "../main.h"
System_t *dmi_read() {
    char *sys_files[]= {"bios_vendor","bios_release","bios_date","bios_version",
    "product_name","product_family","sys_vendor","chassis_vendor"};

    char path[SIZE],buffer[SIZE];
    char *iterate[8];
    System_t *system= malloc(sizeof(System_t));
    for (int i=0;i < sizeof(sys_files) / sizeof(sys_files[0]); i++) {
        snprintf(path,SIZE,"/sys/class/dmi/id/%s",sys_files[i]);
        FILE *sys= fopen(path,"r");
        if (sys == NULL) {
            perror("failed");
            //return NULL;
            continue;
        }
          
        if (fgets(buffer,SIZE,sys) != NULL) {
            iterate[i]= malloc(strlen(buffer)+ 1);
            strcpy(iterate[i],buffer);
            fclose(sys);
        }
    }
    strncpy(system->bios_vendor,iterate[0],sizeof(system->bios_vendor));
    strncpy(system->release,iterate[1],sizeof(system->release));
    strncpy(system->date,iterate[2],sizeof(system->date));
    strncpy(system->version,iterate[3],sizeof(system->version));
    strncpy(system->product_name,iterate[4],sizeof(system->product_name));
    strncpy(system->product_family,iterate[5],sizeof(system->product_family));
    strncpy(system->sys_vendor,iterate[6],sizeof(system->sys_vendor));
    strncpy(system->chassis_vendor,iterate[7],sizeof(system->chassis_vendor));
    for (int i=0;i<8;i++) {
        free(iterate[i]);
    }
    return system;
}
void system_enum() {
    System_t *system= dmi_read();
    if (system != NULL) {
       printf(ANSI_COLOR_LIGHT_GREEN "Bios vendor:"ANSI_COLOR_RESET "\t\t%s",system->bios_vendor);
       printf(ANSI_COLOR_LIGHT_GREEN "Bios: version:"ANSI_COLOR_RESET "\t\t%s\n",system->release);
       printf(ANSI_COLOR_LIGHT_GREEN "Bios release date:"ANSI_COLOR_RESET "\t%s",system->date);
       printf(ANSI_COLOR_LIGHT_GREEN "Product Name:"ANSI_COLOR_RESET "\t\t%s",system->product_name);
       printf(ANSI_COLOR_LIGHT_GREEN "product family:\t\t"ANSI_COLOR_RESET "%s",system->product_family);
       printf(ANSI_COLOR_LIGHT_GREEN "System vendor"ANSI_COLOR_RESET "\t\t%s",system->sys_vendor);
       printf(ANSI_COLOR_LIGHT_GREEN "Chassis Vendor"ANSI_COLOR_RESET "\t\t%s",system->chassis_vendor);
       free(system);
    }
}