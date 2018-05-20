/*
program: file shredder
desc   : securely scramble file without truncating it
author : sandwichdoge@gmail.com
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dirnav.h"


char* generate_random_data(const int nDataLen){
	srand(time(NULL));
	char  _c;
	char* ret = malloc(nDataLen * sizeof(char)); //add 1 for \0 if i want a string
	register int i;

	for(i = 0; i<nDataLen; ++i){
		_c = rand() % (253 + 1 - 1) + 1;
		*(ret+i) = _c;
	}
	//*(ret+nDataLen) = '\0';  //not necessary but jic i want random string with size of $nDataLen

	return ret;
}


char* repeat_char(const char _c, const int nCount, const int isRawData)
{
	char *ret;
	if (isRawData == 1)  //return raw data
		ret = malloc(nCount);
	else  //return null terminated string
		ret = malloc(nCount + 1);

	memset(ret, _c, nCount);
	if (isRawData == 0)
		*(ret+nCount) = '\0';
	return ret;
}


size_t file_get_size(char* sTargetFile)
{
	size_t Sz;
	FILE *_fp;
	_fp = fopen(sTargetFile, "rb");
	if (_fp == NULL)
	{
		printf("problem opening file.\n");
		return 0;
	}
	fseek(_fp, 0, SEEK_END);
	Sz = ftell(_fp);
	fclose(_fp);
	return Sz;  //in bytes
}


int shred_file(char* sTargetFile)
{
	int nBufferSz;
	char* tBuffer;
	size_t Sz = file_get_size(sTargetFile);
	if (Sz == 0)
		return -1;
	size_t Sz_kB = Sz/1024 + (Sz%1024!=0);  //round up, number of buffers needed

	for (int nPass = 1; nPass <= 2; ++nPass)
	{
		int fp = open(sTargetFile, O_WRONLY);
		if (Sz >= 1024)
		{
			for(int i = 1; i<=Sz_kB; ++i)
			{
				if (i==Sz_kB)
					nBufferSz = Sz % 1024;  //exact size of last buffer
				else
					nBufferSz = 1024;
				if     (nPass == 1)       //1st pass = zero data
					tBuffer = repeat_char('\0', nBufferSz, 1);
				else if(nPass == 2)  //2nd pass = write random data
					tBuffer = generate_random_data(nBufferSz);

				write(fp, tBuffer, nBufferSz);
				free(tBuffer);
			}
		}
		else
		{
			tBuffer = generate_random_data(Sz);
			write(fp, tBuffer, Sz);
		}
		close(fp);
	}
	return 0;
}

//[SECTION: CUSTOM FILE OP]
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

void remove_file(char* sTargetPath)
{
	char* sOldFileDir = get_file_dir(sTargetPath, 1);
	char* sNewFileName = repeat_char('a', (int)(strlen(get_file_name(sTargetPath))), 0);
	char* sNewFilePath = str_cat(sOldFileDir, sNewFileName);
	rename(sTargetPath, sNewFilePath);
	if (is_directory(sTargetPath))
		rmdir(sTargetPath);
	else
		remove(sNewFilePath);
	free(sNewFilePath);
	return;
}
//[END OF SECTION: CUSTOM FILE OP]


void shred_dir(char* path, int isRecursive, int isDelete)
{
    //recursive
    DIR *hStream ;
    if (!(hStream = opendir(path)))
        return;
    struct dirent *entry;
    char* sFullEntryName;
    while ((entry = readdir(hStream))){
    sFullEntryName = malloc(strlen(entry->d_name) + strlen(path) + 2);
    strcpy(sFullEntryName, path);
    strcat(sFullEntryName, chr_to_str(c_separator()));
    strcat(sFullEntryName, entry->d_name);
    if ((entry->d_type == DT_DIR) && (isRecursive)){
		if (strcmp(entry->d_name, "..") == 0 || (strcmp(entry->d_name, ".") == 0))
				continue;
		shred_dir(sFullEntryName, isRecursive, isDelete);
    }
    else if (entry->d_type == DT_REG){
		//target entry is a file
        printf("shredding file %s\n", sFullEntryName);
        shred_file(sFullEntryName);
    }
	if (isDelete)
		remove_file(sFullEntryName);
    free(sFullEntryName);

    }
    closedir(hStream);
    return;
}


void show_help(char* command)
{
	printf( "Desc  : Securely erase your data\n"
				"Author: sandwichdoge@gmail.com\n\n"
				"usage     : %s <file_path> [-d]\n"
				"parameters:\n\n"
				"-d:	scramble name and delete target after shredding.\n"
				"-r:	recursive", command);
	return;
}

void main(int argc, char **argv){
	if (argc == 1 || strcmp(argv[1], "--help") == 0 || strstr(argv[1], "?")){
		show_help(argv[0]);
		return;
	}

	char* sTargetPath = argv[1];
	int isRecursive = 0;
	int isDelete = 0;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp("-r", argv[i]) == 0)
			isRecursive = 1;
		if (strcmp("-d", argv[i]) == 0)
			isDelete = 1;
	}

	if (is_directory(sTargetPath)){
		shred_dir(sTargetPath, isRecursive, isDelete);
		printf("directory %s has been shredded, recursive = %d\n", sTargetPath, isRecursive);
		return;
	}

	size_t Sz = file_get_size(sTargetPath);
	if (Sz == 0)
	{
		printf("'%s' does not exist.\n", sTargetPath);
		return;
	}

	printf("shredding file %s\n", sTargetPath);
	printf("%zu bytes\n", Sz);

	if (shred_file(sTargetPath) == -1)
		printf("failed to shred file %s\n", sTargetPath);
	else{
		if (isDelete == 1){  //'-d' as argument -> rename and delete file
			printf("scramling filename and removing file..\n");
			remove_file(sTargetPath);
		}
	}
	printf("finished shredding.\n");
	return;
}
