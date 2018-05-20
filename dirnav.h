#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef stringops
#define stringops
#endif

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

char c_separator()
{
	#ifdef _WIN32  //windows dir separator
		return '\\';
	#else
		return '/';
	#endif
}


char* get_file_dir(char* sFilePath, int isLastSeparatorIncluded)
{
	char* ptr;
	ptr = strrchr(sFilePath, c_separator());
	if (isLastSeparatorIncluded == 1)
		++ptr;  //include last separator e.g. home/user/myfolder/
	size_t _len = strlen(ptr);
	size_t _retSz = strlen(sFilePath) - _len;
	char* ret = malloc(_retSz + 1);
	memcpy(ret, sFilePath, _retSz);
	*(ret+_retSz) = '\0';  //null terminate result string
	return ret;
}


char* get_file_name(char* sFilePath)
{
	char *ptr = malloc(strlen(sFilePath) + 1);
	ptr = strrchr(sFilePath, c_separator());
	++ptr;  //do not include last separator
	return ptr;
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