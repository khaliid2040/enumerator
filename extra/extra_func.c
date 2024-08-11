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
    char path[PATH],contents[SIZE];
    struct dirent *entry;
    struct acpi *node;
    struct acpi *head= NULL;
    DIR *thermal = opendir("/sys/devices/virtual/thermal");
    if (thermal == NULL) {
        perror("dirent");
        return NULL;
    }   
    while ((entry=readdir(thermal)) !=NULL) {
        //pass .. and . directories
        if (!strcmp(entry->d_name,"..") || !strcmp(entry->d_name,".")) {
            continue;
        }
        //open temp and state and save space by overwriting values
        snprintf(path,PATH,"/sys/devices/virtual/thermal/%s/temp",entry->d_name);
        //printf("%s\n",path);
        FILE *tempfp= fopen(path,"r");
        if (tempfp == NULL) {
            continue;
        }
        if (fgets(contents,sizeof(contents),tempfp) == NULL) {
            fclose(tempfp);
            continue;
        }
        node= malloc(sizeof(struct acpi));
        if (node==NULL) {   
            perror("node");
            continue;
        }
        node->next=NULL;
        node->temp= strtoul(contents,NULL,10);
        fclose(tempfp);
        //now overwriting path and content buffers
        snprintf(path,PATH,"/sys/devices/virtual/thermal/%s/mode",entry->d_name);
        //printf("%s\n",path);
        FILE *modefp= fopen(path,"r");
        if (modefp==NULL) {
            free(node);
            continue;
        }
        if (fgets(contents,SIZE,modefp) != NULL) {
            strncpy(node->state,contents,sizeof(node->state));
            node->state[sizeof(node->state) - 3] = '\0'; // Ensure null termination
        }
        
        fclose(modefp);
        node->next=head;
        head= node;
    }
    closedir(thermal);
    return node;
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
        // Print each row with properly aligned columns
        printf("%-10s %-10s %8d °C\n", tempb, current->state, current->temp / 1000);
        
        struct acpi *temp = current;
        current = current->next;
        free(temp);  // Free each node after printing
        count++;
    }     
    return;    
}