#include "../main.h"
//TODOS: need to calculate size dynamically instead of statically printing
int memory_info() {
    struct sysinfo mem;
	if (sysinfo(&mem) == 0) {
        float total_ram = ((float) mem.totalram / 1024 / 1024 /1024);
        float free_ram = ((float) mem.freeram / 1024 / 1024 /1024);
        float shared_ram = ((float) mem.sharedram / 1024 / 1024);
        float total_swap = ((float) mem.totalswap / 1024 / 1024 /1024);
        float free_swap = ((float) mem.freeswap / 1024 / 1024 /1024);	
        printf(ANSI_COLOR_BLUE "Total(GiB)\tFree(GiB)\tShared(MiB)\tTswap(GiB)\t Fswap(GiB)\n"ANSI_COLOR_RESET);
        printf("%.1f\t\t%.1f\t\t%.1f\t\t%.1f\t\t  %.1f",total_ram,free_ram,shared_ram,
        total_swap,free_swap);
		
    }	
} 