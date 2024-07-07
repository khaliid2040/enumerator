#include "../main.h"
System_t *dmi_read() {
    char *sys_files[]= {"bios_vendor","bios_release","bios_date","bios_version",
    "product_name","product_family","sys_vendor","board_vendor"};

    char path[SIZE],buffer[SIZE];
    char *iterate[8];
    System_t *system= malloc(sizeof(System_t));
    for (int i=0;i < sizeof(sys_files) / sizeof(sys_files[0]); i++) {
        snprintf(path,SIZE,"/sys/class/dmi/id/%s",sys_files[i]);
        FILE *sys= fopen(path,"r");
        if (sys == NULL) {
            perror("failed");
            return NULL;
        }
        
        if (fgets(buffer,SIZE,sys) != NULL) {
            iterate[i]= malloc(strlen(buffer));
            strcpy(iterate[i],buffer);
            fclose(sys);
        }
    }
    strcpy(system->bios_vendor,iterate[0]);
    strcpy(system->release,iterate[1]);
    strcpy(system->date,iterate[2]);
    strcpy(system->version,iterate[3]);
    strcpy(system->product_name,iterate[4]);
    strcpy(system->product_family,iterate[5]);
    strcpy(system->sys_vendor,iterate[6]);
    strcpy(system->board_vendor,iterate[7]);
    for (int i=0;i<8;i++) {
        free(iterate[i]);
    }
    return system;
}
void system_enum() {
    System_t *system= dmi_read();
    if (system != NULL) {
       printf("Bios vendor: %s",system->bios_vendor);
       printf("Bios: version: %s\n",system->release);
       printf("Bios release date: %s",system->date);
       printf("MotherBoard vendor: %s",system->board_vendor);
       printf("Product Name: %s",system->product_name);
       printf("product family: %s",system->product_family);
       printf("System vendor %s",system->sys_vendor);
       free(system);
    }
}