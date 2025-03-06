#include "../main.h"

// Get memory information from /proc/meminfo
static double get_memory_info(const char *key, char *unit, size_t len) {
    FILE *fp;
    char *line = NULL;
    size_t size = 0;
    double value = 0.0;

    fp = fopen("/proc/meminfo", "r");
    if (!fp)
        return 0.0;

    while (getline(&line, &size, fp) != -1) {
        if (strncmp(key, line, strlen(key)) == 0) {
            sscanf(line + strlen(key), "%lf", &value);
            break;
        }
    }
    free(line); // Free the buffer allocated by getline
    fclose(fp);

    return convert_size_unit(value, unit, len);
}

static double get_available_memory(char *unit, size_t len) {
    return get_memory_info("MemAvailable:", unit, len);
}

static double get_used_memory(char *unit, size_t len) {
    double total_mem = get_memory_info("MemTotal:", unit, len);
    double available_mem = get_memory_info("MemAvailable:", unit, len);
    return total_mem - available_mem;
}

int memory_info() {
    struct sysinfo mem;
    char unit[7][4]; // Separate units for each value
    size_t len = sizeof(unit[0]);

    if (sysinfo(&mem) == -1) {
        perror("sysinfo");
        return -1;
    }

    // Calculate memory values and their respective units
    double total_mem = convert_size_unit(mem.totalram / UNIT_SIZE, unit[0], len);
    double free_mem = convert_size_unit(mem.freeram / UNIT_SIZE, unit[1], len);
    double shared_mem = convert_size_unit(mem.sharedram / UNIT_SIZE, unit[2], len);
    double swap_total = convert_size_unit(mem.totalswap / UNIT_SIZE, unit[3], len);
    double swap_free = convert_size_unit(mem.freeswap / UNIT_SIZE, unit[4], len);
    double available_mem = get_available_memory(unit[5], len);
    double used_mem = get_used_memory(unit[6],len);    
    
    // Print memory information
    printf("Total\t\tUsed\t\tFree\t\tAvailable\t\tShared\t\tSwap Total\t\tSwap Free\n");
    printf("%.1f %-4s\t%.1f %-4s\t%.1f %-4s\t%.1f %-4s\t\t%.1f %-4s\t%.1f %-4s\t\t\t%.1f %-4s\n",
           total_mem, unit[0],
           used_mem, unit[6],  // Reuse the unit from total_mem for consistency
           free_mem, unit[1],
           available_mem, unit[5],
           shared_mem, unit[2],
           swap_total, unit[3],
           swap_free, unit[4]);

    return 0;
}