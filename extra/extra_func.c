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

bool count_processor(int* cores_count, int* processors_count) {
    bool check=false;
    char *cpuinfo_buffer= NULL;
    size_t buffer_size= 0;
    cpuProperty processors = "processor";
    cpuProperty cores = "cores";
    FILE *cpuinfo = fopen("/proc/cpuinfo","r");

    if (cpuinfo == NULL) {
        printf("Failed to open cpuinfo file.\n");

    }
    while (getline(&cpuinfo_buffer, &buffer_size, cpuinfo) != -1)
    {
        if (strstr(cpuinfo_buffer, processors) != NULL) {
            (*processors_count)++;
        }
        if (strstr(cpuinfo_buffer, cores) != NULL) {
            (*cores_count)++;
        }
        check=true;
    }
    fclose(cpuinfo);
    free(cpuinfo_buffer);
    return check;
}