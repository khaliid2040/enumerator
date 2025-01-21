#include "../main.h"
#ifdef SELINUX_H
#include <selinux/selinux.h> 
static void selinux(void) {
    if (is_selinux_enabled()) {
        printf("SELinux: " ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else {
        printf("SELinux: " ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
    }

    FILE *state_file, *mls_file;
    char buffer[1024];

    // Check SELinux enforce status
    state_file = fopen("/sys/fs/selinux/enforce", "r");
    if (state_file == NULL) {
        perror("fopen /sys/fs/selinux/enforce");
        return;
    }

    if (fgets(buffer, sizeof(buffer), state_file) != NULL) {
        bool enforce = atoi(buffer);
        if (enforce) {
            printf("SELinux: " ANSI_COLOR_GREEN "enforce\n" ANSI_COLOR_RESET);
        } else {
            printf("SELinux: " ANSI_COLOR_YELLOW "permissive\n" ANSI_COLOR_RESET);
        }
    } else {
        perror("fgets /sys/fs/selinux/enforce");
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
#elif APPARMOR_H
#include <sys/apparmor.h>
static void apparmor(void) {
    //check if enabled
    if (aa_is_enabled()) {
        printf("Apparmor:\t" ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else {
        printf("Apparmor:\t" ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
        return; // there is nothing to continue apparmor disabled
    }
    char *mnt;
    aa_find_mountpoint(&mnt); //check mountpoint
    unsigned int count=0,estate=0,cstate=0;
    char profile_path[SIZE];
    char *buffer= malloc(SIZE);
    size_t size= sizeof(buffer);
    char state[10],profile[SIZE];
    snprintf(profile_path,SIZE,"%s/profiles",mnt);
    FILE *fp= fopen(profile_path,"r");
    if (fp == NULL) {
        fprintf(stderr,ANSI_COLOR_RED "couldn't open %s\n" ANSI_COLOR_RESET ,profile_path);
        return;
    }
    while (getline(&buffer,&size,fp) != -1) {
        count++;
        if (sscanf(buffer, "%[^ ] (%[^)])", profile, state) == 2) {
            if (!strcmp(state,"enforce")) {
                estate++;
            } else if (!strcmp(state,"complaint")) {
                cstate++;
            }
        }
    }
    printf("\t\tprofiles:\t %d\n",count);
    printf("\t\tenforce:\t %d\n",estate);
    printf("\t\tcomplaint:\t %d\n\n",cstate);
    free(buffer);
    fclose(fp);
}
#endif
//for   checking the Linux security Modules
void LinuxSecurityModule(void) {
    #ifdef APPARMOR_H
    apparmor();
    #elif defined(SELINUX_H)
    selinux();
    #endif
    //also check for others
    char buf[64];
    FILE *fp = fopen("/sys/kernel/security/lsm","r");
    if (!fp) {
        fprintf(stderr,ANSI_COLOR_RED "failed to open /sys/kernel/security/lsm\n"ANSI_COLOR_RESET);
        return;
    }
    if (fgets(buf,sizeof(buf),fp) != NULL) {
        if (strstr(buf, "landlock")) {
            printf("Landlock\t" ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
        } else 
        if (strstr(buf, "bpf")) {
            printf("BPF\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
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
        printf(ANSI_COLOR_RED"secureboot disabled"ANSI_COLOR_RESET);
    } else if (state == 0x1) {
        printf(ANSI_COLOR_GREEN "secureboot enabled"ANSI_COLOR_RESET);
    }
    close(fd);
    return 0;
} 