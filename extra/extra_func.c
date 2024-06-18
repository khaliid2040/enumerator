#include <stdio.h>
#include <string.h>
#include "../main.h"

int process_file(char *path,char *filename) {
    printf(ANSI_COLOR_MAGENTA "%s: " ANSI_COLOR_RESET ,filename );
    FILE *file= fopen(path,"r");
    char file_buff[MAX_LINE_LENGTH];
    if (filename == NULL) {
        fprintf(stderr,"couldn't open the file");
        return 1;
    }
    while (fgets(file_buff,sizeof(file_buff),file) != NULL) {
        file_buff[strcspn(file_buff, "\n")] = '\0';
        printf("%s\n",file_buff);
    }
    fclose(file);
    return 0;

}
//for checking the Linux security Modules
int LinuxSecurityModule() {
    FILE *fp;
    char buffer[SIZE];
    
    // Open the /sys/kernel/securitylsm file for reading
    fp = fopen("/sys/kernel/security/lsm", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read the contents of the file
    if (fgets(buffer, SIZE, fp) != NULL) {
        // Check if SELinux is being used
        if (strstr(buffer, "selinux")) {
            printf(ANSI_COLOR_GREEN "SELinux detected.\n" ANSI_COLOR_RESET);
        } 
        // Check if AppArmor is being used
        else if (strstr(buffer, "apparmor")) {
            printf(ANSI_COLOR_GREEN "AppArmor detected.\n" ANSI_COLOR_RESET);
        } 
        // If neither SELinux nor AppArmor is found, print unknown
        else {
            printf(ANSI_COLOR_RED "Unknown LSM is in use.\n" ANSI_COLOR_RESET);
        }
    } else {
        printf(ANSI_COLOR_RED "Unable to determine the LSM.\n" ANSI_COLOR_RESET);
    }

    // Close the file
    fclose(fp);

    return 0;

}
//this function is for memory calculation
unsigned long long extract_value(const char* line) {
    // Find the colon (':') character
    const char* colon = strchr(line, ':');
    if (colon == NULL)
        return 0;

    // Move the pointer past the colon
    colon++;

    // Extract the value using strtoull
    return strtoull(colon, NULL, 10);
}
// and this one is for round
double round_to_nearest_tenth(double value) {
    return round(value * 10.0) / 10.0;
}
//to convert size dynamically
double convert_size(double size_kib) {
    extern const char* unit;

    if (size_kib >= 1024.0) {
        size_kib /= 1024.0;
        unit = "MiB";

        if (size_kib >= 1024.0) {
            size_kib /= 1024.0;
            unit = "GiB";
        }
    }
    //printf("Size: %.2f %s\n", size_kib, unit);
    return size_kib;
}