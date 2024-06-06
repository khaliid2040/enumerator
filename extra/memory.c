#include "../main.h"
int memory_info() {
    FILE *fp;
    char buffer[255];
    unsigned long long total_mem = 0, free_mem = 0, buffer_cache = 0;

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
            buffer_cache += extract_value(buffer);
        }
    }

    fclose(fp);

    // Convert memory sizes to gibibytes and round to the nearest tenth
    double total_mem_gib = round_to_nearest_tenth(total_mem / (1024.0 * 1024.0));
    double free_mem_gib = round_to_nearest_tenth(free_mem / (1024.0 * 1024.0));
    double buffer_cache_gib = round_to_nearest_tenth(buffer_cache / (1024.0 * 1024.0));
    double used_mem_gib = round_to_nearest_tenth(total_mem_gib - free_mem_gib - buffer_cache_gib);

    struct MemInfo memory;
    memory.total_mem= total_mem_gib;
    memory.free_mem= free_mem_gib;
    memory.buf_cache_mem= buffer_cache_gib;
    memory.used_mem= used_mem_gib;
    // Output the results
    printf("Total Memory: %.1f GiB\n", memory.total_mem);
    printf("Free Memory: %.1f GiB\n", memory.free_mem);
    printf("Buffer/Cache: %.1f GiB\n", memory.buf_cache_mem);
    printf("Used Memory: %.1f GiB\n", memory.used_mem);
}