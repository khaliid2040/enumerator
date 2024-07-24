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

