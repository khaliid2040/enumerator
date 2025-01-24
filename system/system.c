#include "../main.h"

/*
  * I noticed on some systems the program will just crash due to some files missing results the iterate array
  * to be null strncpy() always dereferences the source so it will dereference the null pointer so before we pass we 
  * need to verify if iterate array is valid if not then abort and just return
*/
static bool verify_data(const char *data) {
    if (!data) return false;
    return true;
}

static System_t *dmi_read() {
    char *sys_files[]= {"bios_vendor","bios_release","bios_date","bios_version",
    "product_name","product_family","sys_vendor","chassis_vendor"};

    char path[SIZE],buffer[SIZE];
    char *iterate[8];
    System_t *system= malloc(sizeof(System_t));
    for (int i=0;i < sizeof(sys_files) / sizeof(sys_files[0]); i++) {
        iterate[i] = malloc(1); // pre allocate the memory so strncpy don't dereference a null pointer
        snprintf(path,SIZE,"/sys/class/dmi/id/%s",sys_files[i]);
        FILE *sys= fopen(path,"r");
        if (sys == NULL) {
            perror("failed");
            //return NULL;
            continue;
        }
          
        if (fgets(buffer,SIZE,sys) != NULL) {
            iterate[i]= realloc(iterate[i],strlen(buffer)+2);
            strcpy(iterate[i],buffer);
            fclose(sys);
        }
    }
    for (int i=0; i<7; i++) {
        if (!verify_data(iterate[i])) {
            fprintf(stderr,ANSI_COLOR_RED"Fatal: Encoutered Unexpected error:invalid array index %d\n"ANSI_COLOR_RESET,i);
            return NULL;
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

#ifdef LIBSENSORS
bool get_sensors_information() {
    const sensors_chip_name *chip, *chip_names = NULL;
    const sensors_feature *features;
    const sensors_subfeature *subfeature;
    const char *chip_name;
    char *label;
    int nr = 0, feature_nr = 0;
    double temp;

    // Initialize the sensors library
    if (sensors_init(NULL) != 0) {
        fprintf(stderr, "Failed to initialize sensors library\n");
        return false;
    }

    printf("Detected Sensors:\n");
    printf("=================\n");

    // Iterate through all detected sensor chips
    while ((chip = sensors_get_detected_chips(chip_names, &nr)) != NULL) {
        chip_name = sensors_get_adapter_name(&chip->bus);
        if (!chip_name) {
            fprintf(stderr, "Failed to retrieve chip adapter name\n");
            continue;
        }
        printf("Adapter: %s\n", chip_name);

        // Iterate through all features of the chip
        feature_nr = 0;
        while ((features = sensors_get_features(chip, &feature_nr)) != NULL) {
            // Check if the feature is related to temperature
            if (features->type != SENSORS_FEATURE_TEMP)
                continue;

            label = sensors_get_label(chip, features);

            // Replace temp[n] with "Temperature" unless it's a core label
            char formatted_label[256];
            if (strncmp(label, "temp", 4) == 0) {
                snprintf(formatted_label, sizeof(formatted_label), "Temperature");
            } else {
                snprintf(formatted_label, sizeof(formatted_label), "%s", label);
            }

            // Get the subfeature (e.g., input temperature)
            subfeature = sensors_get_subfeature(chip, features, SENSORS_SUBFEATURE_TEMP_INPUT);
            if (!subfeature) {
                fprintf(stderr, "Failed to retrieve subfeature for %s\n", features->name);
                continue;
            }

            // Retrieve the sensor value
            if (sensors_get_value(chip, subfeature->number, &temp) < 0) {
                fprintf(stderr, "Failed to get value for %s\n", features->name);
                continue;
            }
            free(label);
            // Print formatted label and temperature
            printf("\t%s:\t+%.1f°C\n", formatted_label, temp);
        }

        printf("\n");
    }

    // Cleanup and release resources
    sensors_cleanup();
    return true;
}
#endif