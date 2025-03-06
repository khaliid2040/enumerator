#include "../main.h"

//source of power related information fetching
static char* get_value(const char* device, const char* file) {
    FILE *fp;
    char path[MAX_PATH];
    char *content = malloc(32);
    if (!content) return NULL;
    
    snprintf(path, sizeof(path), "%s/%s/%s", BATTERY_PATH, device, file);
    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Error: Couldn't open %s: %s\n", path, strerror(errno));
        free(content);
        return NULL;
    }

    if (fgets(content, 32, fp) == NULL) {
        fclose(fp);
        free(content);
        return NULL;
    }
    fclose(fp);

    content[strcspn(content, "\n")] = '\0'; // Remove newline
    return content;
}

static int get_value_number(const char* device, const char* file) {
    FILE *fp;
    char path[MAX_PATH];
    int value = -1;

    snprintf(path, sizeof(path), "%s/%s/%s", BATTERY_PATH, device, file);
    fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Error: Couldn't open %s: %s\n", path, strerror(errno));
        return -1;
    }

    if (fscanf(fp, "%d", &value) != 1) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return value;
}

static void get_battery(const char* battery) {
    int capacity, present, voltage;
    char *vendor, *model, *technology, *status, *serial;

    printf(DEFAULT_COLOR "Battery:\t\t" ANSI_COLOR_RESET "%s\n", battery);

    // Manufacturer
    vendor = get_value(battery, "manufacturer");
    printf(DEFAULT_COLOR "Vendor:\t\t\t" ANSI_COLOR_RESET "%s\n", vendor ? vendor : "Unknown");

    // Model
    model = get_value(battery, "model_name");
    printf(DEFAULT_COLOR "Model:\t\t\t" ANSI_COLOR_RESET "%s\n", model ? model : "Unknown");

    // Technology
    technology = get_value(battery, "technology");
    printf(DEFAULT_COLOR "Technology:\t\t" ANSI_COLOR_RESET "%s\n", technology ? technology : "Unknown");

    // Serial
    serial = get_value(battery, "serial_number");
    printf(DEFAULT_COLOR "Serial:\t\t\t" ANSI_COLOR_RESET "%s\n", serial ? serial : "Unknown");

    // Status
    status = get_value(battery, "status");
    capacity = get_value_number(battery, "capacity");
    printf(DEFAULT_COLOR "Status:\t\t\t" ANSI_COLOR_RESET "%d%% %s\n", capacity, status ? status : "Unknown");

    // Present
    present = get_value_number(battery, "present");
    printf(DEFAULT_COLOR "Present:\t\t" ANSI_COLOR_RESET "%s\n",
           present == 1 ? "Yes" : present == 0 ? "No" : "Unknown");

    // Voltage (converted to volts)
    voltage = get_value_number(battery, "voltage_now");
    if (voltage > 0)
        printf(DEFAULT_COLOR "Voltage:\t\t" ANSI_COLOR_RESET "%.3f V\n", voltage / 1e6);
    else
        printf(DEFAULT_COLOR "Voltage:\t\t" ANSI_COLOR_RESET "Unknown\n");

    // Free allocated memory
    free(vendor);
    free(model);
    free(technology);
    free(serial);
    free(status);
}

static void get_ac_info(const char* ac) {
    char *type;
    int online;
    printf(DEFAULT_COLOR "\nDevice:\t\t\t" ANSI_COLOR_RESET "%s\n", ac);
    type = get_value(ac, "type");
    printf(DEFAULT_COLOR "Type:\t\t\t" ANSI_COLOR_RESET "%s\n", type ? type : "Unknown");
    online = get_value_number(ac, "online");
    printf(DEFAULT_COLOR "Online:\t\t\t" ANSI_COLOR_RESET "%s\n",
           online == 1 ? "Yes" : online == 0 ? "No" : "Unknown");
    free(type);
}

void print_battery_information() {
    DIR *dir;
    struct dirent *entry;
    printf(ANSI_COLOR_YELLOW "Checking for power devices\n" ANSI_COLOR_RESET);

    // Check if the directory is empty
    if (is_directory_empty(BATTERY_PATH)) {
        fprintf(stderr, ANSI_COLOR_RED "No power devices detected\n" ANSI_COLOR_RESET);
        return;
    }

    dir = opendir(BATTERY_PATH);
    if (!dir) {
        fprintf(stderr, ANSI_COLOR_RED "Error: Couldn't get battery information: %s\n" ANSI_COLOR_RESET, strerror(errno));
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (!strncmp(entry->d_name, "BAT", 3))
            get_battery(entry->d_name);
        if (!strncmp(entry->d_name, "AC", 2))
            get_ac_info(entry->d_name);
    }
    closedir(dir);
}