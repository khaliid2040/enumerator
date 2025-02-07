#ifndef PCI_H
#define PCI_H

#include "../main.h"

#define PCI_DEVICE_PATH "/sys/bus/pci/devices"
/*supply raw vendor id and class id then it will return a pointer that is 
 * pointing to a null terminated string identifying the given name. Caller should free the memory
 * implemented on system/pci.c
*/
char* find_device_name(const char *vendor_id, const char *device_id);

void list_pci_devices();

#endif //PCI_H