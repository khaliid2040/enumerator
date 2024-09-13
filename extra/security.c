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
void apparmor(void) {
    //check if enabled
    if (aa_is_enabled()) {
        printf("Apparmor:\t" ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else {
        printf("Apparmor:\t" ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
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
    printf("profiles:\t %d\n",count);
    printf("enforce:\t %d\n",estate);
    printf("complaint:\t %d\n",cstate);
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
            printf("Tomoyo\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
        if (strstr(buf, "capability")) {
            printf("Capability\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
        if (strstr(buf, "lockdown")) {
            printf("Lockdown\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
        if (strstr(buf, "yama")) {
            printf("Yama\t" ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
        }
    }
}

#ifdef LIBEFI
int GetSecureBootStatus ()
{
 unsigned char *data = NULL;
    size_t size;
    unsigned int attributes;
    unsigned int secureboot = -1;
    unsigned int state = -1;

    if (efi_get_variable (efi_guid_global, "SecureBoot", &data, &size,&attributes) < 0) {
            fprintf(stderr,"Secure boot not supported\n");
            return 1;
    }   

    if (size == 4 || size == 2 || size == 1) {
        secureboot = 0;
        memcpy(&secureboot, data, size);
    }
    free(data); 

    data = NULL;
    if (efi_get_variable (efi_guid_shim, "MokSBStateRT", &data, &size,&attributes) >= 0) {
        state = 1;
        free (data);
    }
    if (secureboot) {
        printf(ANSI_COLOR_GREEN "secureboot enabled"ANSI_COLOR_RESET);
        return 1;
    } else {
        printf(ANSI_COLOR_RED "secureboot disabled");
        return 0;
    }   

        return 0;   
}
#endif  