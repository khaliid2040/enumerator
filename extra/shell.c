#include "../main.h"

static int get_shell_type_comm(enum Shell *sh) {
    FILE *fp;
    char content[32], path[MAX_PATH];

    snprintf(path, sizeof(path), "/proc/%d/comm", getppid());
    fp = fopen(path, "r");
    if (!fp) return -1;
    if (fgets(content, sizeof(content), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    if (strcmp(content, "bash\n") == 0) {
        *sh = Bash;
    } else if (strcmp(content, "zsh\n") == 0) {
        *sh = Zsh;
    } else if (strcmp(content, "fish\n") == 0) {
        *sh = Fish;
    } else if (strcmp(content, "csh\n") == 0) {
        *sh = Csh;
    } else { // like if we run on sudo
        return -1;
    }
}

static void get_shell_type(enum Shell *sh) {
    DIR *dp;
    struct dirent *entry;
    FILE *fp;
    char path[96], content[64];
    int pid;

    // If there is no debugger, assume the shell is the parent
    if (!is_debugger_present()) {
        if (get_shell_type_comm(sh) != -1) return;
    }

    dp = opendir("/proc");
    if (!dp) return;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (!is_pid_directory(entry->d_name)) continue;
        pid = atoi(entry->d_name);
        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        fp = fopen(path, "r");
        if (!fp) continue;
        if (fgets(content, sizeof(content), fp) == NULL) {
            fclose(fp);
            continue;
        }
        fclose(fp);
        if (strcmp(content, "bash\n") == 0) {
            *sh = Bash;
            break;
        } else if (strcmp(content, "zsh\n") == 0) {
            *sh = Zsh;
            break;
        } else if (strcmp(content, "fish\n") == 0) {
            *sh = Fish;
            break;
        } else if (strcmp(content, "csh\n") == 0) {
            *sh = Csh;
            break;
        }
    }
    closedir(dp);
}

static void get_bash_version(char *version) {
    FILE *fp = popen("bash -i -c 'echo $BASH_VERSION'", "r");
    if (!fp) return;
    if (fgets(version, VERSION_LEN, fp) != NULL) version[strcspn(version, "\n")] = '\0';
    pclose(fp);
}

static void get_zsh_version(char *version) {
    FILE *fp = popen("zsh -i -c 'echo $ZSH_VERSION'", "r");
    if (!fp) return;
    if (fgets(version, VERSION_LEN, fp) != NULL) version[strcspn(version, "\n")] = '\0';
    pclose(fp);
}

static void get_fish_version(char *version) {
    FILE *fp = popen("/usr/bin/fish -c 'echo $version'", "r");
    if (!fp) {
        version[0] = '\0';
        return;
    }
    if (fgets(version, VERSION_LEN, fp) != NULL)
        version[strcspn(version, "\r\n")] = '\0';  // Remove newlines
    pclose(fp);
}

void get_shell_version(char *version, enum Shell *sh) {
    get_shell_type(sh);
    switch (*sh) {
        case Bash:
            get_bash_version(version);
            break;
        case Fish:
            get_fish_version(version);
            break;
        case Csh:
            break;
        case Zsh:
            get_zsh_version(version);
            break;
    }
}
