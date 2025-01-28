#include "../main.h"
#include <sys/statvfs.h>
#include <mntent.h>

//this function is exported used in main.c
long long get_disk_size(const char* device) {
    char path[MAX_PATH];
    unsigned long long size;
    char content[32];
    FILE *fp;

    snprintf(path,MAX_PATH,"/sys/block/%s/size",device);
    fp = fopen(path,"r");
    if (!fp) return 0;
    if (fgets(content,sizeof(content),fp) == NULL) return 0;
    fclose(fp);
    size = strtoull(content,NULL,0);
    return size * 512;
}
/*
 * reference and explaination refer to 
 * MBR: https://en.wikipedia.org/wiki/Master_boot_record
 * GPT  https://en.wikipedia.org/wiki/GUID_Partition_Table
*/
static void get_partition_table(const char *device) {
    int fd;
    unsigned char buffer[512];
    char path[MAX_PATH];

    snprintf(path,sizeof(path),"/dev/%s",device);
    printf(DEFAULT_COLOR"Partition table:\t"ANSI_COLOR_RESET);
    fd = open(path,O_RDONLY,0644);
    if (fd == -1) {
        printf("N/A\n");
        return;
    }
    if (pread(fd,buffer,sizeof(buffer),0) == -1) {
        printf("N/A\n");
        close(fd);
        return;
    }
    if (buffer[510] == 0x55 && buffer[511] == 0xAA && buffer[450] != 0xEE) {
        printf("MBR/DOS\n");
    }

    if (pread(fd,buffer,sizeof(buffer),512) == -1) {
        printf("N/A\n");
        close(fd);
        return;
    }
    if (!memcmp(buffer,"EFI PART",8)) {
        printf("GPT\n");
        return;
    }

    close(fd);
}

static void get_disk(const char* device) {
    // Get disk model
    char model[64] = {0};
    const char *paths[] = {
        "/sys/block/nvme0n1/device/model",
        "/sys/block/sda/device/model"
    };
    FILE *fp = NULL;

    printf(DEFAULT_COLOR "Model\t\t\t" ANSI_COLOR_RESET);

    if(!strcmp("nvme0n1",device)) {
        fp = fopen(paths[0],"r");
        
        if (fp) {
            if (fgets(model,sizeof(model),fp) == NULL)
                strncpy(model,"unknown",sizeof(model));
            fclose(fp);
        } else {
            strncpy(model,"unknown",sizeof(model));
        }

    } else if (!strcmp("sda",device)) {
        fp = fopen(paths[1],"r");
        if (fp) {
            if (fgets(model,sizeof(model),fp) == NULL)
                strncpy(model,"unknown",sizeof(model));
            fclose(fp);
        } else {
            strncpy(model,"unknown",sizeof(model));
        }
    }

    printf("%s",model);

}
static int get_device_uuid(const char *device,char* uuid, size_t size) {
    DIR *dir;
    struct dirent *entry;
    char path[96], buffer[120];
    const char *partition;
    dir = opendir("/dev/disk/by-uuid");
    if (!dir) {
        perror("opendir");
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) {
            continue;
        }
        snprintf(path, sizeof(path), "/dev/disk/by-uuid/%s", entry->d_name);
        ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
        if (len == -1) {
            closedir(dir);
            return 0;
        }

        buffer[len] = '\0';  // Null-terminate the buffer
        partition = device + 5;
        // Check if the partition matches the current symlink target
        if (strstr(buffer, partition)) {
            strncpy(uuid, entry->d_name, size);
            closedir(dir);
            return 1; // UUID matched
        }
    }

    closedir(dir);
    return 0; // No match found
}
/* getting all the device and there sizes this is independent from the filesystem information 
*  using information from mtent*/
#ifdef LIBUDEV
#include <libudev.h>
static void process_udev() {
    struct udev *udev;
    struct udev_device *dev;
    struct udev_enumerate *udev_enum;
    struct udev_list_entry *list_entry, *devices;

    // Initialize udev
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, ANSI_COLOR_RED "Error: failed initializing udev\n" ANSI_COLOR_RESET);
        return;
    }

    // Create enumerate object
    udev_enum = udev_enumerate_new(udev);
    if (!udev_enum) {
        fprintf(stderr, ANSI_COLOR_RED "Error: failed to create enumerate object\n" ANSI_COLOR_RESET);
        udev_unref(udev);
        return;
    }

    udev_enumerate_add_match_subsystem(udev_enum, "block");
    udev_enumerate_scan_devices(udev_enum);

    list_entry = udev_enumerate_get_list_entry(udev_enum);
    if (!list_entry) {
        fprintf(stderr, ANSI_COLOR_RED "Error: failed to get list entry\n" ANSI_COLOR_RESET);
        udev_enumerate_unref(udev_enum);
        udev_unref(udev);
        return;
    }
    printf("\n"); // make a new line b/w model and printing device for clarity
    udev_list_entry_foreach(devices, list_entry) {
        const char *path, *tmp;
        unsigned long long disk_size = 0;
        unsigned short int block_size = 0;

        path = udev_list_entry_get_name(devices);
        dev = udev_device_new_from_syspath(udev, path);
        if (!dev) {
            fprintf(stderr, ANSI_COLOR_RED "Error: failed to get device from syspath\n" ANSI_COLOR_RESET);
            continue;
        }
        // prevent printing partitions
        if (!strcmp("partition",udev_device_get_devtype(dev)) || !strncmp("loop",udev_device_get_sysname(dev),4)
            || !strncmp("sr",udev_device_get_sysname(dev),2)) {
            udev_device_unref(dev);
            continue;   
        }
        printf(DEFAULT_COLOR "Node:\t\t\t"ANSI_COLOR_RESET "%s\n", udev_device_get_devnode(dev));
        printf(DEFAULT_COLOR "Device:\t\t\t" ANSI_COLOR_RESET "%s\n", udev_device_get_sysname(dev));
        get_disk(udev_device_get_sysname(dev));
        get_partition_table(udev_device_get_sysname(dev));
        // Now getting disk size
        tmp = udev_device_get_sysattr_value(dev, "size");
        if (tmp) {
            disk_size = strtoull(tmp, NULL, 10);
        }

        tmp = udev_device_get_sysattr_value(dev, "queue/logical_block_size");
        if (tmp) {
            block_size = (unsigned short int)atoi(tmp);
        }

        printf(DEFAULT_COLOR "SIZE:\t\t\t"ANSI_COLOR_RESET);
        if (strncmp(udev_device_get_sysname(dev), "sr", 2) != 0) {
            printf("%lld GB\n\n", (disk_size * block_size) / 1000000000);
        } else {
            printf("n/a\n");
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(udev_enum);
    udev_unref(udev);
} 
#endif

void storage(void) {
        // Additional processing if LIBUDEV is enabled
#ifdef LIBUDEV
    process_udev();
#endif
    // Open the mount points file
    FILE *mtab = setmntent("/etc/mtab", "r");
    if (mtab != NULL) {
        // Print table header
        printf("%-16s%-16s%-16s% -16s%-16s%-16s%-1s\n",
               "Device", "Filesystem", "Mountpoint", "Size",
               "Free", "Used", "UUID");

        char *needed_filesystem[] = {"ext4", "ext3", "ext2", "btrfs", "vfat", "exfat"};
        struct mntent *entry;

        // Read each entry from the mount points file
        while ((entry = getmntent(mtab)) != NULL) {
            struct statvfs fs_info;
            if (statvfs(entry->mnt_dir, &fs_info) == 0) {
                // Calculate sizes in KiB (since convert_size_unit expects KiB)
                unsigned long long block_size_kib = fs_info.f_frsize / 1024;
                unsigned long long total_size_kib = fs_info.f_blocks * block_size_kib;
                unsigned long long free_blocks_kib = fs_info.f_bfree * block_size_kib;
                unsigned long long used_blocks_kib =
                    (fs_info.f_blocks - fs_info.f_bfree) * block_size_kib;

                // Convert sizes to human-readable units
                char unit_total[4], unit_free[4], unit_used[4];
                double total_size = convert_size_unit(total_size_kib, unit_total, sizeof(unit_total));
                double free_blocks = convert_size_unit(free_blocks_kib, unit_free, sizeof(unit_free));
                double used_blocks = convert_size_unit(used_blocks_kib, unit_used, sizeof(unit_used));

                // Check if filesystem type is needed
                int needed = 0;
                for (int i = 0; i < sizeof(needed_filesystem) / sizeof(needed_filesystem[0]); i++) {
                    if (strcmp(entry->mnt_type, needed_filesystem[i]) == 0) {
                        needed = 1;
                        break;
                    }
                }

                // Retrieve UUID if BLKID is enabled
                char uuid[96];
                get_device_uuid(entry->mnt_fsname,uuid,sizeof(uuid));
                // Print only if needed
                if (needed) {
                    printf("%-15.15s% -15.12s%-15.12s%10.2f %-4s %10.2f %-4s %10.2f %-4s %s\n",
                            entry->mnt_fsname, entry->mnt_type, entry->mnt_dir,
                            total_size, unit_total,
                            free_blocks, unit_free,
                            used_blocks, unit_used,
                            uuid ? uuid : "N/A");

                }
            }
        }
        // Close the mount points file
        endmntent(mtab);
    }
}
