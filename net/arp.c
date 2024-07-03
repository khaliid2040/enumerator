#include "../main.h"
void arp(void) {
    FILE *arp_file= fopen("/proc/net/arp","r");
    char arp_buf[SIZE];
    char ip_address[16];
    char hw_type[4];
    char flags[8];
    char hw_address[18];
    char mask[12];
    char device[16];
    fgets(arp_buf,SIZE,arp_file);
    while (fgets(arp_buf,SIZE,arp_file) != NULL) {
        sscanf(arp_buf,"%s %s %s %s %s %s",ip_address,hw_type,flags,hw_address,mask,device);
    }
    printf("%-8s  %-16s  %-16s\n","Device","address","mac");
    printf("%-8s  %-16s  %-16s\n",device, ip_address, hw_address);
    fclose(arp_file);   
}