#include "../main.h"
#ifdef SELINUX_H
#include <selinux/selinux.h> 
void selinux(void) {
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
            printf("SELinux: " ANSI_COLOR_GREEN "MLS enabled\n" ANSI_COLOR_RESET);
        } else {
            printf("SELinux: " ANSI_COLOR_YELLOW "MLS disabled\n" ANSI_COLOR_RESET);
        }
    } else {
        perror("fgets /sys/fs/selinux/mls");
    }
    fclose(mls_file);
}
#elif APPARMOR_H
#include <sys/apparmor.h>
void apparmor(void) {
    //check if enabled
    if (aa_is_enabled()) {
        printf("Apparmor:" ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else {
        printf("Apparmor: " ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
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
    printf("profiles: %d\n",count);
    printf("enforce: %d\n",estate);
    printf("complaint: %d\n",cstate);
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
    #else
    printf(ANSI_COLOR_RED "No LSM found\n" ANSI_COLOR_RESET);
    #endif  
}
