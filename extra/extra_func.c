#include <stdio.h>
#include <string.h>
#include "../main.h"

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
