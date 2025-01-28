#ifndef SYSTEM_H
#define SYSTEM_H
#include "../main.h"

//used by extra/system.c
typedef struct {
    char bios_vendor[SIZE];
    char release[9];
    char date[15];
    char version[10];
    char product_name[SIZE];
    char product_family[SIZE];
    char sys_vendor[SIZE];
    char chassis_vendor[SIZE];
} System_t;



//used in extra_fun.c
struct acpi {
    char type[10];
    char state[10];
    float temp;
    struct acpi *next;
};


void gpu_info();

//for accessing function implemented in extra/exra_func.c
void get_pci_info(void);


void system_enum(void);

void acpi_info(void);

void trim_whitespace(char *str);
#endif // SYSTEM_H