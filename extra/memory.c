#include "../main.h"  // Include your header file containing struct definitions and function prototypes
const char* unit;

int memory_info() {
    FILE *fp;
    char buffer[255];
    unsigned long long total_mem = 0, free_mem = 0, buffer_mem = 0,available_mem=0;
    double human_total_mem,human_free_mem,human_buffer,human_avialable;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    // Iterate over each line in the file
    while (fgets(buffer, 255, fp)) {
        // Check for the lines containing required information
        if (strstr(buffer, "MemTotal:") != NULL) {
            total_mem = extract_value(buffer);
        } else if (strstr(buffer, "MemFree:") != NULL) {
            free_mem = extract_value(buffer);
        } else if (strstr(buffer, "Buffers:") != NULL || strstr(buffer, "Cached:") != NULL) {
            // Both "Buffers" and "Cached" contribute to buffer/cache
            buffer_mem += extract_value(buffer);
        } else if (strstr(buffer, "MemAvailable:")) {
            available_mem += extract_value(buffer);
        }
    }

    fclose(fp);
    human_total_mem= convert_size(total_mem);
    human_free_mem= convert_size(free_mem);
    human_buffer= convert_size(buffer_mem);
    human_avialable= convert_size(available_mem);
    printf("%-10s: %8.2f %s\n", "Total", human_total_mem, unit);
    printf("%-10s: %8.2f %s\n", "Free", human_free_mem, unit);
    printf("%-10s: %8.2f %s\n", "Buffer", human_buffer, unit);
    printf("%-10s: %8.2f %s\n", "Available", human_avialable, unit);
    return 0;
}
