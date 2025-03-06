#include "../main.h"

static bool is_apparmor_enabled(bool *loaded) {
    bool (*aa_is_enabled)(void);
    bool enabled;

    if(loaded) *loaded = true;
    if (access("/sys/kernel/security/apparmor",F_OK) == -1) {
        if (loaded) *loaded = false;
        return false;
    }

    void* handle = load_library("libapparmor.so", "aa_is_enabled", (void**)&aa_is_enabled);
    if (!handle) {
        handle = load_library("libapparmor.so.1", "aa_is_enabled", (void**)&aa_is_enabled);
        if (!handle) return false;
    }
    enabled = aa_is_enabled();
    dlclose(handle);
    return enabled;
}

static bool is_selinux_enabled(bool *loaded) {
    void* handle;
    bool enabled;
    bool (*_is_selinux_enabled)(void);
    if (loaded) *loaded = true;
    if (access("/sys/fs/selinux",F_OK) == -1){
        if (loaded) *loaded = false;
        return false;
    }
    handle = load_library("libselinux.so","is_selinux_enabled",(void**)&_is_selinux_enabled);
    if (!handle) {
        // one more attempt
        handle = load_library("libselinux.so.1","is_selinux_enabled",(void**)&_is_selinux_enabled);
        if (!handle) return false;
    }
    enabled = _is_selinux_enabled();
    dlclose(handle);
    return enabled;
}

static bool is_selinux_mls_enabled() {
    void* handle;
    bool enabled;
    bool (*_is_selinux_mls_enabled) (void);

    handle = load_library("libselinux.so","is_selinux_mls_enabled",(void**)&_is_selinux_mls_enabled);
    if (!handle) {
        //one more attempt
        handle = load_library("libselinux.so.1","is_selinux_mls_enabled",(void**)&_is_selinux_mls_enabled);
        if (!handle) return false;
    }
    enabled = _is_selinux_mls_enabled();
    dlclose(handle);
    return enabled;
}

static void selinux(void) {
    FILE *state_file, *mls_file;
    char buffer[1024];

    if (!is_selinux_enabled(NULL) && !is_selinux_mls_enabled()) return;
    // Check SELinux enforce status
    state_file = fopen("/sys/fs/selinux/enforce", "r");
    if (state_file == NULL) {
        perror("fopen /sys/fs/selinux/enforce");
        return;
    }

    if (fgets(buffer, sizeof(buffer), state_file) != NULL) {
        bool enforce = atoi(buffer);
        if (enforce) {
            printf("SELinux:\t" ANSI_COLOR_GREEN "enforce\n" ANSI_COLOR_RESET);
        } else {
            printf("SELinux:\t" ANSI_COLOR_YELLOW "permissive\n" ANSI_COLOR_RESET);
        }
    } else {
        perror("Error reading /sys/fs/selinux/enforce");
    }
    fclose(state_file);

    // Check SELinux MLS status
    mls_file = fopen("/sys/fs/selinux/mls", "r");
    if (mls_file == NULL) {
        perror("fopen /sys/fs/selinux/mls");
        return;
    }

    if (fgets(buffer, sizeof(buffer), mls_file) != NULL) {
        bool mls_enabled = atoi(buffer);
        if (mls_enabled) {
            printf("SELinux:\t" ANSI_COLOR_GREEN "MLS enabled\n" ANSI_COLOR_RESET);
        } else {
            printf("SELinux:\t" ANSI_COLOR_YELLOW "MLS disabled\n" ANSI_COLOR_RESET);
        }
    } else {
        perror("fgets /sys/fs/selinux/mls");
    }
    fclose(mls_file);
}

static void apparmor(void) {
    char *buffer = NULL;
    unsigned int count = 0, estate = 0, cstate = 0;
    char profile[SIZE], state[SIZE];
    size_t size;

    if (is_apparmor_enabled(NULL)) { // this time loaded and enabled
        printf("Apparmor:\t"ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
    } else {
        return;
    }

    FILE *fp = fopen("/sys/kernel/security/apparmor/profiles", "r");
    if (fp == NULL) {
        fprintf(stderr, ANSI_COLOR_RED "couldn't open /sys/kernel/security/apparmor %s\n" ANSI_COLOR_RESET, strerror(errno));
        return;
    }
    while (getline(&buffer, &size, fp) != -1) {
        count++;
        if (sscanf(buffer, "%[^ ] (%[^)])", profile, state) == 2) {
            if (!strcmp(state, "enforce")) {
                estate++;
            } else if (!strcmp(state, "complain")) {
                cstate++;
            }
        }
    }
    printf("\t\tprofiles:\t %d\n", count);
    printf("\t\tenforce:\t %d\n", estate);
    printf("\t\tcomplain:\t %d\n\n", cstate);
    free(buffer);
    fclose(fp);
}

//for   checking the Linux security Modules
void LinuxSecurityModule(void) {
    selinux();
    apparmor();
    //also check for others
    char buf[64];
    FILE *fp = fopen("/sys/kernel/security/lsm", "r");
    if (!fp) {
        fprintf(stderr, ANSI_COLOR_RED "failed to open /sys/kernel/security/lsm\n" ANSI_COLOR_RESET);
        return;
    }
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, "landlock")) {
            printf("Landlock\t" ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
        } else 
if (strstr(buf, "bpf")) {
            printf("BPF\t\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
if (strstr(buf, "tomoyo")) {
            printf("Tomoyo\t\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
if (strstr(buf, "capability")) {
            printf("Capability\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
if (strstr(buf, "lockdown")) {
            printf("Lockdown\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
if (strstr(buf, "yama")) {
            printf("Yama\t\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
    }
    fclose(fp);
}

int GetSecureBootStatus() {
    int fd;
    unsigned char state; // 1 byte indicating if 0x1 enabled on 0x0 disabled

    fd = open("/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c",O_RDONLY, 0644);
    if (fd == -1) return -1;
    if (pread(fd,&state,1,4) == -1) {
        close(fd);
        return -1;
    }
    if (state == 0x0) {
        printf(ANSI_COLOR_RED"secureboot disabled\n"ANSI_COLOR_RESET);
    } else if (state == 0x1) {
        printf(ANSI_COLOR_GREEN "secureboot enabled\n"ANSI_COLOR_RESET);
    }
    close(fd);
    return 0;
}
