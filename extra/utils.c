#include "../main.h"

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

void* load_library(const char *library, const char* symbol, void **function) {
    void* handle = dlopen(library, RTLD_LAZY);
    if (!handle) {
        return NULL;
    }
    
    *function = dlsym(handle, symbol);
    if (!*function) {
        fprintf(stderr, "Error: %s\n", dlerror());
        dlclose(handle);
        return NULL;
    }
    return handle;
}

bool is_directory_empty(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (!dir) {
        perror("opendir");
        return false; // Assume it's not empty if it can't be opened
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && 
            (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue; // Skip "." and ".."
        }
        closedir(dir);
        return false; // Directory is not empty
    }

    closedir(dir);
    return true; // Directory is empty
}

//caller should free memory
char* find_device_name(const char *vendor_id, const char *device_id) {
    FILE *fp = NULL;
    char *device = NULL;
    static const char *pci_ids_paths[] = {
        "/usr/share/hwdata/pci.ids",
        "/usr/share/misc/pci.ids",
        "/var/lib/pciutils/pci.ids",
        NULL  // Sentinel value
    };

    // Remove "0x" prefix if present
    if (vendor_id[0] == '0' && vendor_id[1] == 'x') vendor_id += 2;
    if (device_id[0] == '0' && device_id[1] == 'x') device_id += 2;

    // Try opening pci.ids from known locations
    for (int i = 0; pci_ids_paths[i] != NULL; i++) {
        fp = fopen(pci_ids_paths[i], "r");
        if (fp) break;
    }
    if (!fp) return NULL;  // No valid pci.ids file found

    char line[256];
    int vendor_found = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Ignore comments
        if (line[0] == '#') continue;

        // Strip newline
        line[strcspn(line, "\n")] = 0;

        // If the line is not indented, check if it's a vendor
        if (line[0] != '\t') {
            if (strncmp(line, vendor_id, strlen(vendor_id)) == 0 && line[strlen(vendor_id)] == ' ') {
                vendor_found = 1;  // Start looking for device IDs under this vendor
            } else {
                vendor_found = 0;  // Reset if a new vendor appears
            }
        } 
        // If vendor is found, check device ID
        else if (vendor_found && line[0] == '\t') {
            if (strncmp(line + 1, device_id, strlen(device_id)) == 0 && line[strlen(device_id) + 1] == ' ') {
                // Allocate memory for device name
                device = malloc(strlen(line + strlen(device_id) + 2) + 1);
                if (!device) {
                    fprintf(stderr, ANSI_COLOR_RED "Error: Failed to allocate memory: %s\n" ANSI_COLOR_RESET, strerror(errno));
                    fclose(fp);
                    return NULL;
                }
                // Copy the device name (skipping ID and space)
                strcpy(device, line + strlen(device_id) + 2);
                fclose(fp);
                return device;
            }
        }
    }

    fclose(fp);
    return NULL;  // No matching device found
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
