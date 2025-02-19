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

bool is_init_systemd() {
    FILE *fp;
    char content[15];
    fp = fopen("/proc/1/comm","r");
    if (!fp) return false;
    if (fgets(content,sizeof(content),fp) == NULL) {fclose(fp); return false;}
    if (!strcmp(content,"systemd\n")) {
        fclose(fp);
        return true;
    }
    fclose(fp);
    return false;
}
/* detect systemd version and return version string to location pointed by version paramter
 * on success return 0 and on error return -1
 * check if init system is systemd before calling those functions
 */
#ifdef SYSTEMD
int get_systemd_version(char **version) {
    sd_bus *bus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    int r;
    const char *temp_version;  // Temporary pointer

    // Connect to the system bus
    r = sd_bus_default_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return -1;
    }

    // Get systemd version property
    r = sd_bus_get_property(bus,
                            "org.freedesktop.systemd1",
                            "/org/freedesktop/systemd1",
                            "org.freedesktop.systemd1.Manager",
                            "Version",
                            &error,
                            &reply,
                            "s");
    if (r < 0) {
        fprintf(stderr, "Failed to get systemd version: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return -1;
    }

    // Read the version as string
    r = sd_bus_message_read(reply, "s", &temp_version);
    if (r < 0) {
        fprintf(stderr, "Failed to read version response: %s\n", strerror(-r));
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return -1;
    }

    // Allocate memory for the version string and copy it
    *version = strdup(temp_version);
    if (!*version) {
        fprintf(stderr, "Failed to allocate memory for version string\n");
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return -1;
    }

    // Clean up
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);

    return 0;
}


/* get how many units systemd aware of 
 * on success return exact number of units and on failure return 0
*/

int get_systemd_units() {
    sd_bus *bus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    int count = 0;

    // Connect to the system bus
    int r = sd_bus_default_system(&bus);
    if (r < 0) {
        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        return count;
    }

    // Call ListUnits on systemd D-Bus API
    r = sd_bus_call_method(bus,
                           "org.freedesktop.systemd1",           // Service
                           "/org/freedesktop/systemd1",         // Object path
                           "org.freedesktop.systemd1.Manager",  // Interface
                           "ListUnits",                         // Method
                           &error,
                           &reply,
                           "");
    if (r < 0) {
        fprintf(stderr, "Failed to call ListUnits: %s\n", error.message);
        sd_bus_error_free(&error);
        sd_bus_unref(bus);
        return count;
    }

    // Read array of structures from the reply
    r = sd_bus_message_enter_container(reply, 'a', "(ssssssouso)");
    if (r < 0) {
        fprintf(stderr, "Failed to parse response\n");
        sd_bus_message_unref(reply);
        sd_bus_unref(bus);
        return count;
    }

    // Count units
    count = 0;
    while ((r = sd_bus_message_skip(reply, "(ssssssouso)")) > 0) {
        count++;
    }

    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return count;
}
#endif

static int read_first_line(const char *path, char *buf, size_t size) {
    FILE *file = fopen(path, "r");
    if (!file) return 0;
    if (fgets(buf, size, file)) {
        buf[strcspn(buf, "\n")] = '\0'; // Remove newline
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

void detect_sensors() {
    struct dirent *entry;
    DIR *dp; 
    
    printf("Detected Sensors:\n=================\n");
    //we must check if the directory is empty; if it is empty then there is no sensors
    if (is_directory_empty(HWMON_PATH)) {
        printf(ANSI_COLOR_RED "No sensors detected, make sure driver are loaded\n"ANSI_COLOR_RESET);
        return;
    }
    dp = opendir(HWMON_PATH);
    if (!dp) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Skip hidden entries

        char name_path[256], label_path[256], input_path[256];
        char sensor_name[64];
        
        snprintf(name_path, sizeof(name_path), "%s%s/name", HWMON_PATH, entry->d_name);
        if (!read_first_line(name_path, sensor_name, sizeof(sensor_name)))
            continue;

        // Print adapter based on sensor name
        if (strcmp(sensor_name, "coretemp") == 0)
            printf("Adapter: ISA adapter\n");
        else if (strcmp(sensor_name, "acpitz") == 0)
            printf("Adapter: ACPI interface\n");
        else if (strncmp(sensor_name, "nvme", 4) == 0)
            printf("Adapter: PCI adapter\n");
        else
            printf("Adapter: Virtual device\n");

        for (int i = 1; i <= 10; i++) { // Scan up to 10 possible temperature sensors
            snprintf(label_path, sizeof(label_path), "%s%s/temp%d_label", HWMON_PATH, entry->d_name, i);
            snprintf(input_path, sizeof(input_path), "%s%s/temp%d_input", HWMON_PATH, entry->d_name, i);

            char label[64];
            int temp;

            FILE *label_file = fopen(label_path, "r");
            FILE *input_file = fopen(input_path, "r");

            if (input_file && (fscanf(input_file, "%d", &temp) == 1)) {
                if (label_file && fgets(label, sizeof(label), label_file)) {
                    label[strcspn(label, "\n")] = '\0'; // Remove newline
                    printf("        %s:\t%.1f°C\n", label, temp / 1000.0);
                } else {
                    printf("        Temperature:\t%.1f°C\n", temp / 1000.0);
                }
            }

            if (label_file) fclose(label_file);
            if (input_file) fclose(input_file);
        }

        printf("\n"); // Separate sensor groups
    }

    closedir(dp);
}
