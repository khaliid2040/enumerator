#include "../main.h"
#include <sys/statvfs.h>
void storage() {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    char mount_point[MAX_LINE_LENGTH];
    char device[MAX_LINE_LENGTH];
    // Open /proc/mounts file
    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        perror("Error opening /proc/mounts");
    }
    /* first it will iterate all mountpoints then compare with expected mountpoint if so store it in 
    in needed_mountpoints then process with statvfs*/
    // Read each line and extract mount point and device

    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%s %s", device, mount_point);
        char *needed_mount_point[12]= {"/","/boot","/efi","/boot/efi"};
        // statvfs structure and path
        struct statvfs fs_info;
        printf("%-15s%-12s%-12s%-12s\n", "Mountpoint", "Size(GiB)", "Free(GiB)", "Used(GiB)");
        for (int i=0; i<4; i++) {
            if (access(needed_mount_point[i],F_OK) != -1) {
                if (statvfs(needed_mount_point[i], &fs_info) == 0) {
                    unsigned long long block_size = fs_info.f_frsize ? fs_info.f_frsize : fs_info.f_bsize; // Get the block size
                    unsigned long long total_size = fs_info.f_blocks * block_size; // Total size in bytes
                    unsigned long long free_blocks = fs_info.f_bfree * block_size; // Free blocks in bytes
                    unsigned long long used_blocks = total_size - free_blocks; // Used blocks in bytes
                    double total_size_gib = total_size / (double)(1 << 30); // Convert to GiB
                    double free_blocks_gib = free_blocks / (double)(1 << 30); // Convert free blocks to GiB
                    double used_blocks_gib = used_blocks / (double)(1 << 30); // Convert used blocks to GiB
                    printf("%-15s%-12.2f%-12.2f%-12.2f\n", needed_mount_point[i], total_size_gib, free_blocks_gib, used_blocks_gib);
                }
            }
            
        }
        break;
    }

    // Close file
    fclose(fp);
}