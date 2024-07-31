#include "../main.h"
#ifdef APPARMOR_H
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
//for checking the Linux security Modules
void LinuxSecurityModule(void) {
    #ifdef APPARMOR_H
    apparmor();
    #elif defined(SELINUX)
    if (is_selinux_enabled()) {
        printf("SELinux: " ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else if(!(is_selinux_enabled())) {
       printf("Apparmor: " ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "SELinux unknown\n" ANSI_COLOR_RESET  );
    }
    #else
    printf(ANSI_COLOR_RED "No LSM found\n" ANSI_COLOR_RESET);
    #endif  
}
