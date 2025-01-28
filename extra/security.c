#include "../main.h"

static bool is_apparmor_enabled() {
    FILE *fp;
    char content[5];

    
    fp = fopen("/sys/module/apparmor/parameters/enabled","r");
    if (!fp) return false;
    if (fgets(content,sizeof(content),fp) == NULL) {fclose(fp); return false;}

    if (!strcmp(content,"Y"))
        return true;
    if (!access("/sys/kernel/security/apparmor",F_OK)) return true;
}

static void selinux(void) {
    /**  we don't need to verify if selinux is enabled and loaded 
      * because it is already handled in config.sh script so we just to see the state if it is enforcing to permissive
      * if selinux disabled then this function shouldn't be executed. However if selinux is enabled then there could be 
      * in two state either disabled or enbled.*/
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
            printf("SELinux:\t" ANSI_COLOR_GREEN "enforce\n" ANSI_COLOR_RESET);
        } else {
            printf("SELinux:\t" ANSI_COLOR_YELLOW "permissive\n" ANSI_COLOR_RESET);
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

static void apparmor(void) {
    char *buffer;
    unsigned int count,estate,cstate;
    char profile[SIZE],state[SIZE];
    size_t size;
    
    if (is_apparmor_enabled()) {
        printf("Apparmor:\t"ANSI_COLOR_GREEN "enabled\n"ANSI_COLOR_RESET);
    } else{
        printf("Apparmor:\t"ANSI_COLOR_RED "disabled\n"ANSI_COLOR_RESET);
        return; // it is disabled nothing to do
    }
    
    FILE *fp= fopen("/sys/kernel/security/apparmor/profiles","r");
    if (fp == NULL) {
        fprintf(stderr,ANSI_COLOR_RED "couldn't open /sys/kernel/security/apparmor %s\n" ANSI_COLOR_RESET ,strerror(errno));
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

//for   checking the Linux security Modules
void LinuxSecurityModule(void) {
    #ifdef SELINUX_H
    selinux();
    #elif defined(APPARMOR_H)
    apparmor();
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