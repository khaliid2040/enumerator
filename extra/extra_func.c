#include "../main.h"

#define LENGTH 1024 //used by get_pci_info
int process_file(char *path,char *filename) {
    printf(DEFAULT_COLOR "%-20s: " ANSI_COLOR_RESET ,filename );
    FILE *file= fopen(path,"r");
    char file_buff[MAX_LINE_LENGTH];
    if (file == NULL) {
        fprintf(stderr,"couldn't open the file");
        return 1;
    }
    while (fgets(file_buff,sizeof(file_buff),file) != NULL) {
        file_buff[strcspn(file_buff, "\n")] = '\0';
        //printf("%-500s\n",file_buff);
        printf("%s\n", file_buff); 
    }
    fclose(file);
    return 0;

}

//used by main.c:Systeminfo
int is_pid_directory(const char *name) {
    if (name == NULL || *name == '\0') {
        return 0;
    }
    while (*name) {
        if (!isdigit((unsigned char)*name)) {
            return 0;
        }
        name++;
    }
    return 1;
}

#ifdef LIBPCI
void gpu_info(char *model,char *vendor,size_t len) {
    struct pci_access *pac= pci_alloc();
    pci_init(pac);
    pci_scan_bus(pac);
    struct pci_dev *dev;
    for (dev=pac->devices; dev != NULL; dev=dev->next) {
        pci_fill_info(dev,PCI_FILL_BASES | PCI_FILL_IDENT | PCI_FILL_CLASS);
        if (dev->device_class== PCI_CLASS_DISPLAY_VGA ) {
             pci_lookup_name(pac,model,len,PCI_LOOKUP_DEVICE,dev->vendor_id,dev->device_id);
            //manually interpreting vendor id since libpci will give us long info
            switch (dev->vendor_id) {
                case 0x8086: //intel
                    strcpy(vendor,"Intel");
                    break;
                case 0x10DE: //Invidia
                    strcpy(vendor,"INVIDIA");
                    break;
                case 0x1002:
                    strcpy(vendor,"AMD");
                    break;
                case 0x102B:
                    strcpy(vendor,"METROX");
                    break;
                default:
                    strcpy(vendor,"Unknown");
                    break;  
            }
        } 
    }
    pci_cleanup(pac);
}   
void get_pci_info(void) {
    struct pci_access *pac= pci_alloc();
    if (pac == NULL) {
        perror("pci_access");   
        return;
    }
    pci_init(pac);  
    pci_scan_bus(pac);
    unsigned int read;
    char deviceName[LENGTH],class[LENGTH],vendor[LENGTH],buf[LENGTH];
    struct pci_dev *dev,*pci_dev=pac->devices;
    printf("%-35s%-40s%-40s\n","Vendor","Class","Device");
    for (dev=pci_dev;dev!=NULL;dev=dev->next) {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);	/* Fill in header info we need */  
        pci_lookup_name(pac,vendor,LENGTH,PCI_LOOKUP_VENDOR,dev->vendor_id);
        printf("%-30s",vendor);     			
        pci_lookup_name(pac,class,LENGTH,PCI_LOOKUP_CLASS,dev->device_class);
        printf("%-35s",class);
        pci_lookup_name(pac,deviceName,LENGTH,PCI_LOOKUP_DEVICE,dev->vendor_id,dev->device_id);
        printf("%-20s\n",deviceName);
    }
    pci_cleanup(pac);
}
#endif
