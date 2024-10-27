#include "../main.h"
#define PATH 64
#define LENGTH 1024 //used by get_pci_info
int process_file(char *path,char *filename) {
    printf(DEFAULT_COLOR "%-20s: " ANSI_COLOR_RESET ,filename );
    FILE *file= fopen(path,"r");
    char file_buff[MAX_LINE_LENGTH];
    if (file == NULL) {
        fprintf(stderr,"couldn't open the file");
        return 1;
    }
    while (fgets(file_buff,sizeof(file_buff),file) != NULL) {
        file_buff[strcspn(file_buff, "\n")] = '\0';
        //printf("%-500s\n",file_buff);
        printf("%s\n", file_buff); 
    }
    fclose(file);
    return 0;

}

//used by main.c:Systeminfo
int is_pid_directory(const char *name) {
    if (name == NULL || *name == '\0') {
        return 0;
    }
    while (*name) {
        if (!isdigit((unsigned char)*name)) {
            return 0;
        }
        name++;
    }
    return 1;
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
void trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Null terminate after the last non-space character
    *(end + 1) = '\0';
}

#ifdef LIBPCI
void gpu_info(char *model,char *vendor) {
    struct pci_access *pac= pci_alloc();
    pci_init(pac);
    pci_scan_bus(pac);
    struct pci_dev *dev;
    for (dev=pac->devices; dev != NULL; dev=dev->next) {
        pci_fill_info(dev,PCI_FILL_BASES | PCI_FILL_IDENT | PCI_FILL_CLASS);
        if (dev->device_class== PCI_CLASS_DISPLAY_VGA ) {
             pci_lookup_name(pac,model,64,PCI_LOOKUP_DEVICE,dev->vendor_id,dev->device_id);
            //manually interpreting vendor id since libpci will give us long info
            switch (dev->vendor_id) {
                case 0x8086: //intel
                    strcpy(vendor,"Intel");
                    break;
                case 0x10DE: //Invidia
                    strcpy(vendor,"INVIDIA");
                    break;
                case 0x1002:
                    strcpy(vendor,"AMD");
                    break;
                case 0x102B:
                    strcpy(vendor,"METROX");
                    break;
                default:
                    strcpy(vendor,"Unknown");
                    break;  
            }
        } 
    }
    pci_cleanup(pac);
}   
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

        // Read temperature
        snprintf(path, PATH, "/sys/devices/virtual/thermal/%s/temp", entry->d_name);
        FILE *tempfp = fopen(path, "r");
        if (tempfp == NULL) {
            continue;
        }
        if (fgets(contents, sizeof(contents), tempfp) == NULL) {
            fclose(tempfp);
            continue;
        }
        fclose(tempfp);

        // Create new node
        node = malloc(sizeof(struct acpi));
        if (node == NULL) {   
            perror("malloc");
            continue;
        }
        node->next = NULL;
        
        // Store temperature, converting from string to float
        node->temp = strtof(contents, NULL);

        // Read mode (enabled/disabled)
        snprintf(path, PATH, "/sys/devices/virtual/thermal/%s/mode", entry->d_name);
        FILE *modefp = fopen(path, "r");
        if (modefp != NULL) {
            if (fgets(contents, SIZE, modefp) != NULL) {
                trim_whitespace(contents); // Optional: Use a trimming function if needed
                strncpy(node->state, contents, sizeof(node->state) - 1);
                node->state[sizeof(node->state) - 1] = '\0'; // Ensure null termination
            }
            fclose(modefp);
        }

        // Read type of sensor
        snprintf(path, PATH, "/sys/devices/virtual/thermal/%s/type", entry->d_name);
        FILE *typefp = fopen(path, "r");
        if (typefp != NULL) {
            if (fgets(contents, SIZE, typefp) != NULL) {
                trim_whitespace(contents); // Optional: Use a trimming function if needed
                strncpy(node->type, contents, sizeof(node->type) - 1);
                node->type[sizeof(node->type) - 1] = '\0'; // Ensure null termination
            }
            fclose(typefp);
        }

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

    // Print table headers
    printf("%-10s\t%-10s\t%-10s\n", "Sensor", "State", "Temperature");

    while (current != NULL) {

        // Print the state and temperature
        printf("%-10s\t%-10s\t%.1f °C\n", current->type,current->state, current->temp / 1000.0);

        // Move to the next sensor and free the current node
        struct acpi *temp = current;
        current = current->next;
        free(temp);
        count++;
    }     
}
