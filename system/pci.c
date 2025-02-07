#include "../main.h"

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

static char* find_vendor_name(const char *vendor_id) {
    FILE *fp = NULL;
    char *vendor = NULL;
    static const char *pci_ids_paths[] = {
        "/usr/share/hwdata/pci.ids",
        "/usr/share/misc/pci.ids",
        "/var/lib/pciutils/pci.ids",
        NULL  // Sentinel value
    };

    // Remove "0x" prefix if present
    if (vendor_id[0] == '0' && vendor_id[1] == 'x') vendor_id += 2;

    // Try opening pci.ids from known locations
    for (int i = 0; pci_ids_paths[i] != NULL; i++) {
        fp = fopen(pci_ids_paths[i], "r");
        if (fp) break;
    }
    if (!fp) return NULL;  // No valid pci.ids file found

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Ignore comments
        if (line[0] == '#') continue;

        // Strip newline
        line[strcspn(line, "\n")] = 0;

        // Check if the line starts with the vendor ID (not indented)
        if (line[0] != '\t' && strncmp(line, vendor_id, strlen(vendor_id)) == 0 && line[strlen(vendor_id)] == ' ') {
            // Allocate memory for vendor name
            vendor = malloc(strlen(line + strlen(vendor_id) + 1) + 1);
            if (!vendor) {
                fprintf(stderr, "Error: Failed to allocate memory: %s\n", strerror(errno));
                fclose(fp);
                return NULL;
            }
            // Copy the vendor name (skipping ID and space)
            strcpy(vendor, line + strlen(vendor_id) + 1);
            fclose(fp);
            return vendor;
        }
    }

    fclose(fp);
    return NULL;  // No matching vendor found
}

static char* find_class_name(const char *class_id) {
    FILE *fp = NULL;
    char *class_name = NULL;
    static const char *pci_ids_paths[] = {
        "/usr/share/hwdata/pci.ids",
        "/usr/share/misc/pci.ids",
        "/var/lib/pciutils/pci.ids",
        NULL  // Sentinel value
    };

    // Remove "0x" prefix if present
    if (class_id[0] == '0' && class_id[1] == 'x') class_id += 2;

    // Try opening pci.ids from known locations
    for (int i = 0; pci_ids_paths[i] != NULL; i++) {
        fp = fopen(pci_ids_paths[i], "r");
        if (fp) break;
    }
    if (!fp) return NULL;  // No valid pci.ids file found

    char line[256];
    int class_section = 0;
    char base_class[3] = {0};
    char subclass[5] = {0};

    // Extract base class and subclass from the full class ID
    strncpy(base_class, class_id, 2);
    strncpy(subclass, class_id, 4);

    while (fgets(line, sizeof(line), fp)) {
        // Ignore comments
        if (line[0] == '#') continue;

        // Strip newline
        line[strcspn(line, "\n")] = 0;

        // Detect start of class section
        if (strcmp(line, "# List of known classes") == 0) {
            class_section = 1;
            continue;
        }

        // If we are in the class section, search for class ID
        if (class_section) {
            // Check for full class match (BBSSPP)
            if (strncmp(line, class_id, strlen(class_id)) == 0 && line[strlen(class_id)] == ' ') {
                class_name = strdup(line + strlen(class_id) + 1);
                fclose(fp);
                return class_name;
            }
            // Check for subclass match (BBSS)
            else if (strncmp(line, subclass, strlen(subclass)) == 0 && line[strlen(subclass)] == ' ') {
                class_name = strdup(line + strlen(subclass) + 1);
                fclose(fp);
                return class_name;
            }
            // Check for base class match (BB)
            else if (line[0] != '\t' && strncmp(line, base_class, strlen(base_class)) == 0 && line[strlen(base_class)] == ' ') {
                class_name = strdup(line + strlen(base_class) + 1);
                fclose(fp);
                return class_name;
            }
        }
    }

    fclose(fp);
    return NULL;  // No matching class found
}

void list_pci_devices() {
    DIR* dir;
    FILE *fp;
    char vendor[7],class[7],device[7],path[96];
    struct dirent *entry;
    char *found_device = NULL,*found_vendor=NULL,*found_class=NULL;
    dir = opendir(PCI_DEVICE_PATH);
    if (!dir) {
        perror("opendir");
        return;
    }   
    printf("%-35s%-40s\n","Vendor","Device");
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        snprintf(path,sizeof(path),"%s/%s/vendor",PCI_DEVICE_PATH,entry->d_name);
        fp = fopen(path,"r");
        if (!fp) continue;
        if (fgets(vendor,sizeof(vendor),fp) == NULL) { fclose(fp); continue;}
        fclose(fp);

        snprintf(path,sizeof(path),"%s/%s/device",PCI_DEVICE_PATH,entry->d_name);
        fp = fopen(path,"r");
        if (!fp) continue;
        if (fgets(device,sizeof(device),fp) == NULL) {fclose(fp); continue;}
        fclose(fp);

        snprintf(path,sizeof(path),"%s/%s/class",PCI_DEVICE_PATH,entry->d_name);
        fp = fopen(path,"r");
        if (!fp) continue;
        if (fgets(class,sizeof(class),fp) == NULL) {fclose(fp); continue;}
        fclose(fp);

        found_device = find_device_name(vendor,device);
        found_vendor = find_vendor_name(vendor);
        found_class = find_class_name(class);
        
        printf("%-30s %-35s\n",found_vendor,found_device);
        free(found_vendor);
        free(found_device);
        free(found_class);
    }
    closedir(dir);
}
