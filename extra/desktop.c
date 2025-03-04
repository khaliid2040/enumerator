#include "../main.h"
static void get_bash_version(char *version) {
    FILE *fp= popen("bash -i -c 'echo $BASH_VERSION'", "r");
    if (!fp) return;
    if (fgets(version,VERSION_LEN,fp) != NULL) version[strcspn(version, "\n")] = '\0'; 
    fclose(fp);
}
static void get_zsh_version(char* version) {
    FILE *fp = popen("zsh -i -c 'echo $ZSH_VERSION'", "r");
    if (!fp) return;
    if (fgets(version,VERSION_LEN,fp) != NULL) version[strcspn(version, "\n")] = '\0';
    fclose(fp);
}
static void get_fish_version(char *version) {
    FILE *fp = popen("/usr/bin/fish -c 'echo $version'", "r");
    if (!fp) {
        version[0] = '\0';
        return;
    }
    if (fgets(version, VERSION_LEN, fp) != NULL)
        version[strcspn(version, "\r\n")] = '\0';  // Remove newlines
    fclose(fp);
}

static void get_shell_type_comm(enum Shell *sh) {
    FILE *fp;
    char content[32],path[MAX_PATH];

    snprintf(path,sizeof(path),"/proc/%d/comm",getppid());
    fp = fopen(path,"r");
    if (!fp) return;
    if (fgets(content,sizeof(content),fp) == NULL) {fclose(fp); return;}
    fclose(fp);
    if (!strcmp(content,"bash\n")) {
        *sh = Bash;
    }else if (!strcmp(content,"zsh\n")) {
        *sh = Zsh;
    } else if (!strcmp(content,"fish\n")) {
        *sh = Fish;
    } else if (!strcmp(content,"csh\n"))
    {
        *sh = Csh;
    }
    
}
static void get_shell_type(enum Shell *sh) {
    DIR *dp;
    struct dirent *entry;
    FILE *fp;
    char path[96],content[64];
    int pid;
    // if there is no debugger we don't need to waste our time searching for all processes so assume the shell is the parent
    if (!is_debugger_present()) {
        get_shell_type_comm(sh);
        return;
    }

    dp = opendir("/proc");
    if (!dp) return;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if(!is_pid_directory(entry->d_name)) continue;
        #ifdef DEBUG
        pid = atoi(entry->d_name);
        #endif
        snprintf(path,sizeof(path),"/proc/%d/comm",pid);
        fp = fopen(path,"r");
        if (!fp) continue;
        if (fgets(content,sizeof(content),fp) == NULL) {fclose(fp); continue;}
        fclose(fp);
        if (!strcmp(content,"bash\n")) {
            *sh = Bash;
            break;
        } else if (!strcmp(content,"zsh\n")) {
            *sh = Zsh;
            break;
        } else if (!strcmp(content,"fish\n")) {
            *sh = Fish;
            break;
        } else if (!strcmp(content,"csh\n")) {
            *sh = Csh;
            break;
        }
    }
    closedir(dp);
}

void get_shell_version(char *version, enum Shell *sh) {
    char* env;
    get_shell_type(sh);
    switch (*sh) {
        case Bash:
            get_bash_version(version);
            break;
        case Fish:
            get_fish_version(version);
            break;
        case Csh:
        case Zsh:
            get_zsh_version(version);
            break;
    }
}

static void get_kde_version(char* version) {
    FILE *kde;
    char contents[30];
    size_t len= 0;
    kde = popen("plasmashell --version 2>&1","r");
    if (kde) {
        while (fgets(contents,sizeof(contents),kde) != NULL) {
            if (!strncmp(contents,"plasmashell",11)) {
                sscanf(contents,"plasmashell %s",version);
            }
        } 
    } 
}

static void get_gnome_version(char* version) {
    char *contents=NULL;
    FILE *gnome= popen("gnome-shell --version 2>&1","r");
    if (gnome !=NULL) {
        size_t len= 0;
        while (getline(&contents,&len,gnome) != -1) {
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
    if (de) {
        if (!strcmp(de,"KDE")) {
            get_kde_version(version);
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