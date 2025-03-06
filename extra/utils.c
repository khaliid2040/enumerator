#include "../main.h"

#define LENGTH 1024 //used by get_pci_info
int process_file(const char *path, const char *filename) {
    printf(DEFAULT_COLOR "%-20s: " ANSI_COLOR_RESET, filename);
    FILE *file = fopen(path, "r");
    char file_buff[MAX_LINE_LENGTH];
    if (!file) {
        fprintf(stderr, "Couldn't open the file: %s\n", path);
        return 1;
    }
    while (fgets(file_buff, sizeof(file_buff), file) != NULL) {
        file_buff[strcspn(file_buff, "\n")] = '\0';
        printf("%s\n", file_buff); 
    }
    fclose(file);
    return 0;
}

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

bool is_debugger_present() {
    FILE *fp;
    char *content = NULL;
    size_t len = 0;
    unsigned int tpid = 0; 
    char path[64];

    snprintf(path, sizeof(path), "/proc/%d/status", getpid());
    fp = fopen(path, "r");
    if (!fp) {
        perror("Error opening /proc/<pid>/status");
        return false;
    }

    while (getline(&content, &len, fp) != -1) {
        if (strncmp(content, "TracerPid:", 10) == 0) {
            sscanf(content + 10, "%u", &tpid);
            break;
        }    
    }

    fclose(fp);
    free(content);

    return (tpid != 0);
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
