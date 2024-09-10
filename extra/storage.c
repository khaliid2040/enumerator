#include "../main.h"
#include <sys/statvfs.h>
#include <mntent.h>

#ifdef BLKID
#include <blkid/blkid.h>

char *get_uuid(const char *node) {      
    blkid_probe pr;
    const char *uuid = NULL;
    pr = blkid_new_probe_from_filename(node);
    if (!pr) {
        return NULL;
    }

    if (blkid_do_probe(pr) != 0) {
        blkid_free_probe(pr);
        return NULL;
    }

    if (blkid_probe_lookup_value(pr, "UUID", &uuid, NULL) != 0) {
        blkid_free_probe(pr);
        return NULL;
    }

    // Copy the UUID string to a newly allocated buffer
    char *uuid_copy = NULL;
    if (uuid) {
        uuid_copy = strdup(uuid); // Use strdup to allocate and copy the UUID string
        if (!uuid_copy) {
            perror("strdup");
        }
    }

    blkid_free_probe(pr);
    return uuid_copy; // Return the newly allocated copy of the UUID
}
#endif
void storage(void) {
	//get disk model
	    char model[64] = {0};
    const char *paths[] = {"/sys/block/nvme0n1/device/model", "/sys/block/sda/device/model"};
    FILE *fp = NULL;
    int found = 0;

    printf(ANSI_COLOR_LIGHT_GREEN "Model\t\t" ANSI_COLOR_RESET);

    // Try each path to find and read the model information
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        fp = fopen(paths[i], "r");
        if (fp) {
            if (fgets(model, sizeof(model), fp) != NULL) {
                printf("%s", model);  // Model already includes newline
                found = 1;
                fclose(fp);
                break;  // Stop checking other paths if model is found
            }
            fclose(fp);  // Close file if reading failed
        }
    }

    if (!found) {
        printf(ANSI_COLOR_RED "unknown\n" ANSI_COLOR_RESET);
    }	
	
    // Open the mount points file	
    FILE *mtab = setmntent("/etc/mtab", "r");
	if (mtab != NULL) {
    	// Print header
    	printf("%-15s%-12s%-12s%-12s%-12s%-12s%-12s\n", "Device", "Filesystem", "Mountpoint", "Size(GiB)", "Free(GiB)", 
		"Used(GiB)","UUID");
		char *needed_filesystem[6]= {"ext4","ext3","ext2","btrfs","vfat","exfat"};
    	// Read each entry from the mount points file
    	struct mntent *entry;
		int counter=0;
    	while ((entry = getmntent(mtab)) != NULL) {
    	    struct statvfs fs_info;
    	    if (statvfs(entry->mnt_dir, &fs_info) == 0) {
    	        unsigned long long block_size = fs_info.f_frsize ? fs_info.f_frsize : fs_info.f_bsize; // Get the block size
    	        unsigned long long total_size = fs_info.f_blocks * block_size; // Total size in bytes
    	        unsigned long long free_blocks = fs_info.f_bfree * block_size; // Free blocks in bytes
    	        unsigned long long used_blocks = total_size - free_blocks; // Used blocks in bytes
    	        double total_size_gib = total_size / (double)GiB; // Convert to GiB
    	        double free_blocks_gib = free_blocks / (double)GiB; // Convert free blocks to GiB
    	        double used_blocks_gib = used_blocks / (double)GiB; // Convert used blocks to GiB
				int needed=0;
				for (int i=0; i < sizeof(needed_filesystem)/ sizeof(needed_filesystem[0]); i++) {
					if (strcmp(entry->mnt_type,needed_filesystem[i]) ==0) {
						needed=1;
						break;
					}
				}
				//now getting device uuid
				char *uuid= NULL;
				if (needed) {
                    #ifdef BLKID
				    uuid = get_uuid(entry->mnt_fsname);
				    #endif
					printf("%-15s%-12s%-12s%-12.2f%-12.2f%-12.2f%-12s\n", entry->mnt_fsname, entry->mnt_type, 
					entry->mnt_dir, total_size_gib, free_blocks_gib, used_blocks_gib, uuid ? uuid : "N/A");	
				}

    	    }
    	}

    	// Close the mount points file
    	endmntent(mtab);
	}
}