#include "../main.h"

int process_file(char *path,char *filename) {
    printf(ANSI_COLOR_MAGENTA "%-20s: " ANSI_COLOR_RESET ,filename );
    FILE *file= fopen(path,"r");
    char file_buff[MAX_LINE_LENGTH];
    if (file == NULL) {
        fprintf(stderr,"couldn't open the file");
        return 1;
    }
    while (fgets(file_buff,sizeof(file_buff),file) != NULL) {
        file_buff[strcspn(file_buff, "\n")] = '\0';
        //printf("%-500s\n",file_buff);
        printf("%s60\n", file_buff); 
    }
    fclose(file);
    return 0;

}
//for checking the Linux security Modules
void LinuxSecurityModule(void) {
    #ifdef APPARMOR
    if (aa_is_enabled()) {
        printf("Apparmor:" ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else {
        printf("Apparmor: " ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
    }
    #endif
    #ifdef SELINUX
    if (is_selinux_enabled()) {
        printf("SELinux: " ANSI_COLOR_GREEN "enabled\n" ANSI_COLOR_RESET);
    } else if(!(is_selinux_enabled())) {
       printf("Apparmor: " ANSI_COLOR_RED "disabled\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "SELinux unknown\n" ANSI_COLOR_RESET  );
    }
    #endif  
}
//this function is for memory calculation
unsigned long long extract_value(const char* line) {
    // Find the colon (':') character
    const char* colon = strchr(line, ':');
    if (colon == NULL)
        return 0;

    // Move the pointer past the colon
    colon++;

    // Extract the value using strtoull
    return strtoull(colon, NULL, 10);
}
// and this one is for round
double round_to_nearest_tenth(double value) {
    return round(value * 10.0) / 10.0;
}
//to convert size dynamically
double convert_size(double size_kib, char **unit) {
    if (size_kib >= 1024.0) {
        size_kib /= 1024.0;
        *unit = "MiB";

        if (size_kib >= 1024.0) {
            size_kib /= 1024.0;
            *unit= "GiB";
        }
        
    }
    return size_kib;
}