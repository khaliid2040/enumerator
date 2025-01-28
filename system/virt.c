#include "../main.h"

/*on virtualbox i realized things are different cpuid isn't reliable so we need to use different methods*/
static bool is_hypervisor_virtualbox() {
    FILE *fp;
    char buffer[20];

    fp = fopen("/sys/class/dmi/id/board_vendor","r");
    if (!fp)
        return false; //we are in trouble it shouldn't fail we don't have any other hope :(
    if (fgets(buffer,sizeof(buffer),fp) == NULL) {
        fclose(fp);
        return false; // nop also this shouldn't happen again we are in trouble
    }
    if (!strncmp("Oracle",buffer,6)) {
        fclose(fp);
        return true; // finally we got something expected :)
    }
    fclose(fp);
    return false; // we shouldn't reach here
}
//in case we run on docker container
// return 1 for docker, 2 for podman and 0 for none
static int detect_container() {
    FILE *fp;
    char content[64];

    //fallback methods 
    if (!access("/.dockerenv",F_OK)) return 1; //also this is unquestionable just return without further doing nothing

    // on podman /run/.containerenv is enough
    if (!access("/run/.containerenv",F_OK)) return 2;

    //first check on /run/systemd/container
    fp = fopen("/run/systemd/container","r");
    if (!fp) return 0; 

    if (!fgets(content,sizeof(content),fp)) return 0;

    if (!strcmp(content,"docker")) return 1; // this is the most reliable way so without question return true
    fclose(fp);
    return 0; 
}
Virtualization detect_hypervisor() {
    unsigned int eax,ebx,ecx,edx;
    Virtualization virt = none;
    //first check hypervisor presence
    __get_cpuid(1,&eax,&ebx,&ecx,&edx);

    if (ecx & (1 << 31)) {
        printf(DEFAULT_COLOR "\nHypervisor detected:\t"ANSI_COLOR_RESET);

        __cpuid(0x40000000U,eax,ebx,ecx,edx);
        
        // for virtual machines
        if (is_hypervisor_virtualbox()) return Virtualbox;
        if (ebx == 0x4B4D564B && ecx == 0x564B4D56 && edx == 0x0000004D) return KVM;
        if (ebx == 0x61774d56 && ecx == 0x4d566572 && edx == 0x65726177) return Vmware;
        if (ebx == 0x7263694d && ecx == 0x666f736f && edx == 0x76482074) return hyperv;
        if (ebx == 0x566e6558 && ecx == 0x65584d4d && edx == 0x4d4d566e) return xen;

        // for containers
        switch (detect_container()) {
            case 1: virt = docker; break;
            case 2: virt = podman; break;
            default: virt = unknown;
        }
    }
    return virt;
}