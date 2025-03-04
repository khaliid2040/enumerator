#ifndef DESKTOP_H
#define DESKTOP_H
#include "../main.h"

#define VERSION_LEN 7

typedef enum {
    GNOME,
    KDE,
    XFCE,
    MATE,
    NONE
} Desktop;

enum Protocol {
    WAYLAND,
    X11
};
Desktop Detect_desktop(char* version);


struct output_info {
    int x, y;
    int width, height;
    int refresh_rate;
    char *make, *model,*discription;
};

extern struct output_info out_info;

//also system/display.c code will be here
#ifdef LIBWAYLAND
#include <wayland-client.h>
#endif
void get_display_model(enum Protocol p);

// this unction must be called if there is no DE present in the system
// because we assume the parent process is the shell and the shell is the one tight with the user session
// i.e no Desktop envirornment present so be careful
enum Shell{
    Bash,
    Zsh,
    Csh,
    Fish
};
void get_shell_version(char* version,enum Shell *sh);
#endif // DESKTOP_H