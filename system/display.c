#include "../main.h"

/*
        Wayland display detection code

*/

struct output_info out_info; 
// Global variables
static struct wl_display *display;
static struct wl_registry *registry;
static struct wl_output *output = NULL;
#ifdef LIBWAYLAND
// Callback when the compositor announces a new `wl_output` object
static void registry_handler(void *data, struct wl_registry *registry, uint32_t id,
                             const char *interface, uint32_t version) {
    if (strcmp(interface, "wl_output") == 0)
        // Bind to the wl_output object
        output = wl_registry_bind(registry, id, &wl_output_interface, version);
}

// Define the registry listener
static const struct wl_registry_listener registry_listener = {
    registry_handler,
    NULL // No global removal handler
};

// `wl_output` event listener to handle output events
static void output_geometry(void *data, struct wl_output *output, int32_t x, int32_t y,
                            int32_t physical_width, int32_t physical_height, int32_t subpixel,
                            const char *make, const char *model,int32_t w) {
    out_info.x = x;
    out_info.y = y;
    out_info.width = physical_width;
    out_info.height = physical_height;
    out_info.make = strdup(make);
    out_info.model = strdup(model);
}

static void output_mode(void *data, struct wl_output *output, uint32_t flags, int32_t width,
                        int32_t height, int32_t refresh) {
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        out_info.refresh_rate = refresh;
    }
}

// i don't somehow the API is forcing me to setup all callbacks, anyways we don't care about this and we gave it empty callbacks

static void output_done(void *data, struct wl_output *output) {
    // Optional: Handle cleanup if needed
}
static void description(void* data,struct wl_output *output,const char* description) {
    out_info.discription = strdup(description);
}
static void scale(void* data, struct wl_output *output, int factor) {
    
}
static void name(void* data, struct wl_output *output,const char* name) {
    
}
// Correct event listener for `wl_output`
static const struct wl_output_listener output_listener = {
    output_geometry,
    output_mode,
    output_done,
    scale,
    name,
    description,
};

static int Detect_display_wayland() {
        // Connect to the Wayland compositor
        display = wl_display_connect(NULL);
        if (!display) {
            fprintf(stderr, "Failed to connect to the Wayland display\n");
            return -1;
        }
    
        // Get the registry object and add the registry listener
        registry = wl_display_get_registry(display);
        if (!registry) {
            fprintf(stderr, "Failed to get registry\n");
            wl_display_disconnect(display);
            return -1;
        }
        
        wl_registry_add_listener(registry, &registry_listener, NULL);
    
        // Dispatch events to populate the global objects
        int ret = wl_display_dispatch(display);
        if (ret == -1) {
            fprintf(stderr, "Error during dispatch\n");
            wl_display_disconnect(display);
            return -1;
        }
    
        // Add listener to the wl_output object if it was found
        if (output) {
            wl_output_add_listener(output, &output_listener, NULL);
            // Dispatch events to receive output device information
            ret = wl_display_dispatch(display);
            if (ret == -1) {
                fprintf(stderr, "Error during second dispatch\n");
                wl_display_disconnect(display);
                return -1;
            }
        } else {
            fprintf(stderr, "No output device found\n");
        }
        wl_display_disconnect(display);
        return 0;
}
#endif
/*

    X11 display detection to be implemented

*/

void get_display_model(enum Protocol p) {
    switch (p) {
        case WAYLAND:
            #ifdef LIBWAYLAND
            Detect_display_wayland();
            #endif
            break;
        case X11: //currently not implemented
            return;
    }
} 