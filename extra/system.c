#include "../main.h"

static System_t *dmi_read() {
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
        if (iterate[i]) {
         free(iterate[i]);   
        }
    }
    return system;
}
void system_enum() {
    System_t *system= dmi_read();
    if (system != NULL) {
       printf(DEFAULT_COLOR "Bios vendor:"ANSI_COLOR_RESET "\t\t%s",system->bios_vendor);
       printf(DEFAULT_COLOR "Bios: version:"ANSI_COLOR_RESET "\t\t%s\n",system->release);
       printf(DEFAULT_COLOR "Bios release date:"ANSI_COLOR_RESET "\t%s",system->date);
       printf(DEFAULT_COLOR "Product Name:"ANSI_COLOR_RESET "\t\t%s",system->product_name);
       printf(DEFAULT_COLOR "product family:\t\t"ANSI_COLOR_RESET "%s",system->product_family);
       printf(DEFAULT_COLOR "System vendor"ANSI_COLOR_RESET "\t\t%s",system->sys_vendor);
       printf(DEFAULT_COLOR "Chassis Vendor"ANSI_COLOR_RESET "\t\t%s",system->chassis_vendor);
       free(system);
    }
}

static void trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Null terminate after the last non-space character
    *(end + 1) = '\0';
}

static struct acpi* get_acpi() {
    char path[MAX_PATH], contents[SIZE];
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
        snprintf(path, MAX_PATH, "/sys/devices/virtual/thermal/%s/temp", entry->d_name);
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
        snprintf(path, MAX_PATH, "/sys/devices/virtual/thermal/%s/mode", entry->d_name);
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
        snprintf(path, MAX_PATH, "/sys/devices/virtual/thermal/%s/type", entry->d_name);
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
