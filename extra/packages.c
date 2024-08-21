#include "../main.h"
//portage package manager
/*some package manager are written in python like portage in gentoo and dnf in redhat baded distros so if pipe() those package manage
it will slow down the program causing alot of performance digration so it is better if we directly query package database */
int gentoo_pkgmgr() {
    struct dirent *entry;
    char path[100];
    int counter = 0;

    // Open the main directory
    DIR *pkg = opendir("/var/db/pkg");
    if (pkg == NULL) {
        return 0;
    }

    // Loop through entries in the main directory
    while ((entry = readdir(pkg)) != NULL) {
        // Skip the "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the path to the subdirectory
        snprintf(path, sizeof(path), "/var/db/pkg/%s", entry->d_name);

        // Open the subdirectory
        DIR *package = opendir(path);
        if (package == NULL) {
            continue; // Skip this subdirectory and proceed with others
        }

        // Count entries in the subdirectory
        struct dirent *subentry;
        while ((subentry = readdir(package)) != NULL) {
            if (strcmp(subentry->d_name, ".") == 0 || strcmp(subentry->d_name, "..") == 0) {
                continue;
            }
            counter++;
        }

        // Close the subdirectory
        closedir(package);
    }

    // Close the main directory
    closedir(pkg);
    return counter;
}

//debian package manager
int debian_pkgmgr() {
    int counter = 0;
    FILE *pkgmgr;
    char buffer[256];
    pkgmgr = fopen("/var/lib/dpkg/status", "r");
    if (!pkgmgr) {
        perror("Unable to open /var/lib/dpkg/status");
        return -1;
    }
    while (fgets(buffer, sizeof(buffer), pkgmgr)) {
        if (strncmp(buffer, "Package:", 8) == 0) {
            counter++;
        }
    }
    fclose(pkgmgr);
    return counter;
}

//only this package manager are available
int count_flatpak_packages() {
    char buffer[128];
    int count = 0;
    FILE *fp = popen("flatpak list | wc -l", "r");
    if (fp == NULL) {
        perror("popen");
        return -1;
    }

    if (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
        count = atoi(buffer);
    }

    pclose(fp);
    return count;
}
void package_manager() {
    printf(ANSI_COLOR_LIGHT_GREEN "Package:\t"ANSI_COLOR_RESET);
    int packages,flatpak=0;
    #ifdef FLATPAK
    flatpak=count_flatpak_packages();
    #endif
    #ifdef DEBIAN
    packages= debian_pkgmgr();
    printf("%d (apt) %d (flatpak) \n",packages,flatpak);
    #elif GENTOO
    packages= gentoo_pkgmgr();
    printf("%d (emerge) %d (flatpak)\n",packages,flatpak);
    #endif
    
    
}