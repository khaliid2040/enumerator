#include "../main.h"

int memory_info() {
    struct sysinfo mem;
	if (sysinfo(&mem) == 0) {
        float total_ram = ((float) mem.totalram / 1024 / 1024 /1024);
        float free_ram = ((float) mem.freeram / 1024 / 1024 /1024);
        float shared_ram = ((float) mem.sharedram / 1024 / 1024);
        float total_swap = ((float) mem.totalswap / 1024 / 1024 /1024);
        float free_swap = ((float) mem.freeswap / 1024 / 1024 /1024);	
        printf("Total: %6.1f GiB\n", total_ram);
        printf("Free:  %6.1f GiB\n", free_ram);
		printf("Shared: %6.1f MiB\n", shared_ram);
        printf("Tswap:  %6.1f GiB\n", total_swap);
		printf("Fswap: %6.1f GiB\n", free_swap);
		
    }	
} 