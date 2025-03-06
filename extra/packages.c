#include "../main.h"
#include <sys/stat.h>

// Gentoo package manager
static int gentoo_pkgmgr() {
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
static int count_dpkg_packages() {
    FILE *fp;
    char content[64];
    unsigned int count=0;
    fp = fopen("/var/lib/dpkg/status","r");
    if (!fp) return 0;
    while (fgets(content,sizeof(content),fp) != NULL) {
        if (!strncmp(content,"Package:",8)) count++;
    }
    fclose(fp);
    return count;
}

// Red Hat package manager
static int redhat_pkgmgr() {
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
// arch-based distros package manager
static int count_pacman_packages() {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char path[512];
    unsigned int count = 0;

    dir = opendir("/var/lib/pacman/local");
    if (!dir) return 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        // Construct full path and check if it's a directory
        snprintf(path, sizeof(path), "/var/lib/pacman/local/%s", entry->d_name);
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            count++;
        }
    }

    closedir(dir);
    return count;
}
// Flatpak package manager
static int count_flatpak_packages() {
    DIR *dir;
    struct dirent *entry;
    unsigned int count=0;

    dir = opendir("/var/lib/flatpak/runtime/");
    if (!dir) return 0;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) continue;
        count++;
    }
    closedir(dir);
    return count;
}


void package_manager() {
    // Print package counts for defined distros
    #if defined(FLATPAK) || defined(GENTOO) || defined(DEBIAN) || defined(REDHAT) || defined(OPENSUSE)
    printf(DEFAULT_COLOR "Packages:\t"ANSI_COLOR_RESET);
    #endif

    int packages = 0;
    int flatpak = 0;

    #ifdef FLATPAK
    flatpak = count_flatpak_packages();
    #endif

    #ifdef DEBIAN
    packages = count_dpkg_packages();
    printf("%d (apt) ", packages);
    #elif GENTOO
    packages = gentoo_pkgmgr();
    printf("%d (emerge) ", packages);
    #elif REDHAT || OPENSUSE
    packages = redhat_pkgmgr();
    printf("%d (rpm) ", packages);
    #elif ARCH
    packages= count_pacman_packages();
    printf("%d (pacman) ",packages);
    #else
    return;
    #endif

    #ifdef FLATPAK
    printf("%d (flatpak)\n",flatpak);
    #else
    printf("\n");   
    #endif
    
}
