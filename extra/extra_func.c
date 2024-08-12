#include "../main.h"
#define PATH 64
#define LENGTH 1024 //used by get_pci_info
int process_file(char *path,char *filename) {
    printf(ANSI_COLOR_MAGENTA "%-20s: " ANSI_COLOR_RESET ,filename );
    FILE *file= fopen(path,"r");
    char file_buff[MAX_LINE_LENGTH];
    if (file == NULL) {
        fprintf(stderr,"couldn't open the file");
        return 1;
    }
    while (fgets(file_buff,sizeof(file_buff),file) != NULL) {
        file_buff[strcspn(file_buff, "\n")] = '\0';
        //printf("%-500s\n",file_buff);
        printf("%s60\n", file_buff); 
    }
    fclose(file);
    return 0;

}

bool count_processor(int* cores_count, int* processors_count) {
    bool check=false;
    char *cpuinfo_buffer= NULL;
    size_t buffer_size= 0;
    cpuProperty processors = "processor";
    cpuProperty cores = "cores";
    FILE *cpuinfo = fopen("/proc/cpuinfo","r");

    if (cpuinfo == NULL) {
        printf("Failed to open cpuinfo file.\n");

    }
    while (getline(&cpuinfo_buffer, &buffer_size, cpuinfo) != -1)
    {
        if (strstr(cpuinfo_buffer, processors) != NULL) {
            (*processors_count)++;
        }
        if (strstr(cpuinfo_buffer, cores) != NULL) {
            (*cores_count)++;
        }
        check=true;
    }
    fclose(cpuinfo);
    free(cpuinfo_buffer);
    return check;
}
#ifdef LIBPCI
void get_pci_info(void) {
    struct pci_access *pac= pci_alloc();
    if (pac == NULL) {
        perror("pci_access");   
        return;
    }
    pci_init(pac);  
    pci_scan_bus(pac);
    unsigned int read;
    char deviceName[LENGTH],class[LENGTH],vendor[LENGTH],buf[LENGTH];
    struct pci_dev *dev,*pci_dev=pac->devices;
    printf("%-35s%-40s%-40s\n","Vendor","Class","Device");
    for (dev=pci_dev;dev!=NULL;dev=dev->next) {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);	/* Fill in header info we need */  
        pci_lookup_name(pac,vendor,LENGTH,PCI_LOOKUP_VENDOR,dev->vendor_id);
        printf("%-30s",vendor);     			
        pci_lookup_name(pac,class,LENGTH,PCI_LOOKUP_CLASS,dev->device_class);
        printf("%-35s",class);
        pci_lookup_name(pac,deviceName,LENGTH,PCI_LOOKUP_DEVICE,dev->vendor_id,dev->device_id);
        printf("%-20s\n",deviceName);
    }
    pci_cleanup(pac);
}
#endif
struct acpi* get_acpi() {
    char path[PATH], contents[SIZE];
    struct dirent *entry;
    struct acpi *node;
    struct acpi *head = NULL;
    struct acpi *tail = NULL; // To keep track of the end of the list
    DIR *thermal = opendir("/sys/devices/virtual/thermal");
    if (thermal == NULL) {
        perror("opendir");
        return NULL;
    }   

    while ((entry = readdir(thermal)) != NULL) {
        // Skip . and .. directories
        if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) {
            continue;
        }

        // Open temp and state files
        snprintf(path, PATH, "/sys/devices/virtual/thermal/%s/temp", entry->d_name);
        FILE *tempfp = fopen(path, "r");
        if (tempfp == NULL) {
            continue;
        }
        if (fgets(contents, sizeof(contents), tempfp) == NULL) {
            fclose(tempfp);
            continue;
        }

        node = malloc(sizeof(struct acpi));
        if (node == NULL) {   
            perror("malloc");
            fclose(tempfp);
            continue;
        }
        node->next = NULL;
        node->temp = strtoul(contents, NULL, 10);
        fclose(tempfp);

        snprintf(path, PATH, "/sys/devices/virtual/thermal/%s/mode", entry->d_name);
        FILE *modefp = fopen(path, "r");
        if (modefp == NULL) {
            free(node);
            continue;
        }
        if (fgets(contents, SIZE, modefp) != NULL) {
            strncpy(node->state, contents, sizeof(node->state));
            node->state[sizeof(node->state) - 3] = '\0'; // Ensure null termination
        }
        fclose(modefp);

        // Add node to the linked list
        if (head == NULL) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }
    closedir(thermal);
    return head;
}

void acpi_info() {
    struct acpi *head = get_acpi();
    if (head == NULL) {
        fprintf(stderr, "Failed to retrieve ACPI information\n");
        return;
    }

    struct acpi *current = head;
    unsigned int count = 0;
    char tempb[7];

    // Print table headers
    printf("%-10s %-10s %-10s\n", "Sensor", "State", "Temperature");

    while (current != NULL) {
        snprintf(tempb, sizeof(tempb), "temp%u", count);
        printf("%-10s %-10s %8d °C\n", tempb, current->state, current->temp / 1000);
        
        struct acpi *temp = current;
        current = current->next;
        free(temp);  // Free each node after printing
        count++;
    }     
}
  
int GetSecureBootStatus ()
{
        uint8_t *data = NULL;
        size_t data_size;
        uint32_t attributes;
        int32_t secureboot = -1;
        int32_t setupmode = -1;
        int32_t moksbstate = -1;

        if (efi_get_variable (efi_guid_global, "SecureBoot", &data, &data_size,
                              &attributes) < 0) {
                fprintf (stderr, "Failed to read \"SecureBoot\" "
                                 "variable: %m\n");
                return -1;
        }

        if (data_size != 1) {
                printf ("Strange data size %zd for \"SecureBoot\" variable\n",
                        data_size);
        }
        if (data_size == 4 || data_size == 2 || data_size == 1) {
                secureboot = 0;
                memcpy(&secureboot, data, data_size);
        }
        free (data);

        data = NULL;
        if (efi_get_variable (efi_guid_global, "SetupMode", &data, &data_size,
                              &attributes) < 0) {
                fprintf (stderr,ANSI_COLOR_RED "Failed to read \"SetupMode\" "
                                 "variable: %m\n" ANSI_COLOR_RESET);
                return -1;
        }

        if (data_size != 1) {
                printf (ANSI_COLOR_YELLOW "Strange data size %zd for \"SetupMode\" variable\n" ANSI_COLOR_RESET,
                        data_size);
        }
        if (data_size == 4 || data_size == 2 || data_size == 1) {
                setupmode = 0;
                memcpy(&setupmode, data, data_size);
        }
        free (data);

        data = NULL;
        if (efi_get_variable (efi_guid_shim, "MokSBStateRT", &data, &data_size,
                              &attributes) >= 0) {
                moksbstate = 1;
                free (data);
        }

        if (secureboot == 1 && setupmode == 0) {
        printf(ANSI_COLOR_GREEN "SecureBoot enabled\n" ANSI_COLOR_RESET);
        if (moksbstate == 1) {
            printf(ANSI_COLOR_RED "SecureBoot validation is disabled in shim\n" ANSI_COLOR_RESET);
        }
        } else if (secureboot == 0 || setupmode == 1) {
        printf(ANSI_COLOR_RED "SecureBoot disabled\n" ANSI_COLOR_RESET);
        if (setupmode == 1) {
            printf("Platform is in Setup Mode\n");
        }
        } else {
        printf(ANSI_COLOR_YELLOW "Cannot determine secure boot state.\n" ANSI_COLOR_RESET);
        }

        return 0;
}