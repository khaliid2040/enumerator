#include "../main.h"

void print_route_entry(const char *iface, unsigned long dest, unsigned long gateway, unsigned long mask, int flags) {
    char dest_str[INET_ADDRSTRLEN];
    char gateway_str[INET_ADDRSTRLEN];
    char mask_str[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &dest, dest_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &gateway, gateway_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &mask, mask_str, INET_ADDRSTRLEN);

    printf("%-8s  %-16s  %-16s  %-16s  %X\n", iface, dest_str, gateway_str, mask_str, flags);
}

void route(void) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char iface[16];
    unsigned long dest, gateway, mask;
    int flags;

    // Open the route file for reading
    file = fopen("/proc/net/route", "r");
    if (file) {

    // Print the header
    printf("%-8s  %-16s  %-16s  %-16s  %s\n", "Iface", "Destination", "Gateway", "Mask", "Flags");
    printf("------------------------------------------------------------\n");

    // Read each line and parse the fields
    while (fgets(line, MAX_LINE_LENGTH, file)) {
        if (sscanf(line, "%s %lX %lX %X %*s %*s %*s %lX %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s", 
                   iface, &dest, &gateway, &flags, &mask) != 5) {
            continue;
        }

        // Print the parsed route entry
        print_route_entry(iface, dest, gateway, mask, flags);
    }
    fclose(file);
    }
}
