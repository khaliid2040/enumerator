#include "../main.h"
int storage() {
    FILE *fp;
    char line[MAX_LINE_LENGTH];
    char mount_point[MAX_LINE_LENGTH];
    char device[MAX_LINE_LENGTH];
    // Open /proc/mounts file
    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        perror("Error opening /proc/mounts");
        return 1;
    }
    /* first it will iterate all mountpoints then compare with expected mountpoint if so store it in 
    in needed_mountpoints then process with statvfs*/
    // Read each line and extract mount point and device
    struct Mountpoints mounts= {NULL,NULL,NULL,NULL};
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%s %s", device, mount_point);
        char *needed_mount_point[12]= {"/","/boot","/efi","/boot/efi"};
        for (int i=0; i<4; i++) {
            if (access(needed_mount_point[i],F_OK) != -1) {
                if (strcmp(needed_mount_point[i],"/")==0) {
                    mounts.root= strdup(needed_mount_point[i]);
                    printf("%s\n",mounts.root);//successful remove this
                } else if (strcmp(needed_mount_point[i],"/boot")) {
                    mounts.boot= strdup(needed_mount_point[i]);
                    printf("%s\n",mounts.boot);//successful remove this
                } else if (strcmp(needed_mount_point[i],"/boot/efi")) {
                    mounts.efi_boot= strdup(needed_mount_point[i]);
                    printf("%s\n",mounts.efi_boot);//successful remove this
                } else if (strcmp(needed_mount_point[i],"/efi")) {
                    mounts.efi= strdup(needed_mount_point[i]);
                    printf("%s\n",mounts.efi);//successful remove this
                }
            }
            
        }
        break;
        //printf("Mount Point: %s, Device: %s\n", mount_point, device);
    }

    // Close file
    fclose(fp);
    free(mounts.root);
    free(mounts.boot);
    free(mounts.efi_boot);
    free(mounts.efi);

    return 0;
}