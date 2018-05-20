#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "stringops.h"

int is_directory(char* sPath)
{
    DIR* op;
    int ret;
    op = opendir(sPath);
    if (errno == 0)  //is valid dir
        ret = 1;
    else{
        ret = 0;
    } 
    closedir(op);
    return ret;
}


void list_dir(char* path)
{
    //recursive
    printf("listing %s\n", path);
    DIR *hStream ;
    if (!(hStream = opendir(path)))
        return;
    struct dirent *entry;
    char* sFullEntryName;
    while ((entry = readdir(hStream))){
    sFullEntryName = malloc(strlen(entry->d_name) + strlen(path) + 2);
    strcpy(sFullEntryName, path);
    strcat(sFullEntryName, "/");
    strcat(sFullEntryName, entry->d_name);
    if (entry->d_type == DT_DIR){
        if (strcmp(entry->d_name, "..") == 0 || (strcmp(entry->d_name, ".") == 0))
                continue;
        list_dir(sFullEntryName);
        free(sFullEntryName);
    }
    printf("%s\n", sFullEntryName);
    }
    closedir(hStream);
    return;
}
/*
void main(){
    list_dir("/home/z/Lab");
    return;
}
*/