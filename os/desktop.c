#include "../main.h"
//detect the compositor we run on this depend on libwayland-client
// currently worked on gnome mutter and kde kwin.
enum Compositors compositor;

#ifdef LIBWAYLAND
static void registry_handler(void *data, struct wl_registry *registry, uint32_t id,
                             const char *interface, uint32_t version) {
    // Check the specific interfaces you're interested in
    if (strcmp(interface, "zwlr_output_manager_v1") == 0) {
        compositor = SWAY;
    } else if (strcmp(interface, "kde_output_management_v2") == 0) {
        compositor = KWIN;
    } else if (strcmp(interface, "gtk_shell1") == 0) {
        compositor = MUTTER;
    } else if (strcmp(interface, "weston_desktop_shell") == 0) {
        compositor = WESTON;
    }
}

static void registry_remover(void *data, struct wl_registry *registry, uint32_t id) {}

static const struct wl_registry_listener registry_listener = {
    registry_handler,
    registry_remover
};

void detect_compositor() {
    struct wl_display *display = wl_display_connect(NULL);
    if (!display) return;

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    wl_registry_destroy(registry);
    wl_display_disconnect(display);    
}
#endif

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
    pclose(kde);
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