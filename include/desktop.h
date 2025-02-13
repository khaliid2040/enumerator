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

Desktop Detect_desktop(char* version);
#endif // DESKTOP_H