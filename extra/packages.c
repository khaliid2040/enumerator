#include "../main.h"

// Gentoo package manager
int gentoo_pkgmgr() {
    struct dirent *entry;
    char path[256];
    int counter = 0;

    // Open the main directory
    DIR *pkg = opendir("/var/db/pkg");
    if (pkg == NULL) {
        perror("Unable to open /var/db/pkg");
        return -1;
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

// Debian package manager
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

// Red Hat package manager
int redhat_pkgmgr() {
    int counter = 0;
    FILE *pkgmgr;
    char buffer[256];

    pkgmgr = popen("rpm -qa", "r");
    if (!pkgmgr) {
        perror("popen failed");
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), pkgmgr)) {
        // No need to check for "Package:" as rpm -qa only outputs package names
        counter++;
    }
    pclose(pkgmgr);
    return counter;
}

// Flatpak package manager
int count_flatpak_packages() {
    char buffer[128];
    int count = 0;
    FILE *fp = popen("flatpak list | wc -l", "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }

    if (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
        count = atoi(buffer);
    }

    pclose(fp);
    return count;
}

void package_manager() {
    // Print package counts for defined distros
    #if defined(FLATPAK) || defined(GENTOO) || defined(DEBIAN) || defined(REDHAT)
    printf(ANSI_COLOR_LIGHT_GREEN "Packages\t"ANSI_COLOR_RESET);
    #endif

    int packages = 0;
    int flatpak = 0;

    #ifdef FLATPAK
    flatpak = count_flatpak_packages();
    #endif

    #ifdef DEBIAN
    packages = debian_pkgmgr();
    printf("%d (apt) %d (flatpak)\n", packages, flatpak);
    #elif GENTOO
    packages = gentoo_pkgmgr();
    printf("%d (emerge) %d (flatpak)\n", packages, flatpak);
    #elif REDHAT
    packages = redhat_pkgmgr();
    printf("%d (rpm) %d (flatpak)\n", packages, flatpak);
    #else
    printf("No supported package manager defined.\n");
    #endif
}
