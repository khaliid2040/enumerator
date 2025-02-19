#include "../main.h"

static void get_kde_version(char* version,const char* username) {
    char path[40],*contents=NULL    ;
    snprintf(path,sizeof(path),"/home/%s/.config/plasma-welcomerc",username);
    FILE *deVersion= fopen(path,"r");
    if (deVersion !=NULL) {
        size_t len= 0;
        while (getline(&contents,&len,deVersion) != -1) {
            snprintf(version,sizeof(version),"%.6s",contents + 16);  
        }
        fclose(deVersion);
        free(contents);
    } else {
        //if the file doesn't exit print new line character
        printf("\n");
    }
}

static void get_gnome_version(char* version) {
    char *contents=NULL;
    FILE *gnome= popen("gnome-shell --version 2>&1","r");
    if (gnome !=NULL) {
        size_t len= 0;
        if (getline(&contents,&len,gnome) != -1) {
            sscanf(contents + 12, "%s",version);
        }
        pclose(gnome);
        free(contents);
    } else {
        //if the file doesn't exit print new line character
        printf("\n");
    }
}

static void get_xfce_version(char* version) {
    char *content = NULL;
    size_t len =0;
    FILE *pp = popen("xfce4-session --version 2>&1","r");
    if (!pp) return;
    if (getline(&content,&len,pp)) {
        sscanf(content,"xfce4-session  %s",version);
        free(content);
    }
    pclose(pp);
} 

static void get_mate_version(char* version) {
    char* content = NULL;
    size_t len =0;
    FILE *pp = popen("mate-session --version 2>&1","r");
    if (!pp) return;
    if (getline(&content,&len,pp)) {
        sscanf(content,"mate-session %s",version);
        free(content);
    }
    pclose(pp);
}
Desktop Detect_desktop(char* version) {
    char* de = getenv("XDG_CURRENT_DESKTOP");
    struct passwd *pwd = getpwuid(getuid());
    if (de) {
        if (!strcmp(de,"KDE")) {
            get_kde_version(version,pwd->pw_name);
            return KDE;
        } else if (!strcmp(de,"GNOME") || strstr(de,"GNOME")) {
            get_gnome_version(version);
            return GNOME;
        } else if (!strcmp(de,"XFCE")) {
            get_xfce_version(version);
            return XFCE;
        } else if (!strcmp(de,"mate")) {
            get_mate_version(version);
            return MATE;
        }
    } 
    return NONE;
}