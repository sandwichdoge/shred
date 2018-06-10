/*
program: file shredder
desc   : securely scramble file/directory without truncating it
author : sandwichdoge@gmail.com
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../lib/stringops.h"
#include "../lib/dirnav.h"


char* generate_random_data(const int nDataLen){
	//TODO: better randomization, possibly more patterns
	srand(time(NULL));
	char  _c;
	char* ret = malloc(nDataLen);
	register int i;

	for(i = 0; i<nDataLen; ++i){
		_c = rand() % (253 + 1 - 1) + 1;
		*(ret+i) = _c;
	}

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


void sremove(char* sTargetPath)
{
	//scramble file/dir's name before deleting it.
	char* sOldFileDir = extract_file_dir(sTargetPath, 1);
	char* sNewFileName = repeat_char('a', (int)(strlen(extract_file_name(sTargetPath))), 0);
	char* sNewFilePath = str_cat(sOldFileDir, sNewFileName);
	rename(sTargetPath, sNewFilePath);
	if (is_directory(sNewFilePath))
		rmdir(sNewFilePath);
	else
		remove(sNewFilePath);
	free(sNewFilePath);
	return;
}


int shred_file(char* sTargetFile)
{
	int nBufferSz;
	char* tBuffer;
	unsigned int Sz = file_get_size(sTargetFile);
	if (Sz == 0)
		return -1;
	unsigned int Sz_kB = Sz/1024 + (Sz%1024!=0);  //round up, number of buffers needed

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

int shred_dir(char* path, int isRecursive, int isDelete)
{
    //recursive
    DIR *hStream ;
    if (!(hStream = opendir(path)))
        return -1;
    struct dirent *entry;
    char* sFullEntryName;
	int ret;
    while ((entry = readdir(hStream))){
		sFullEntryName = malloc(strlen(entry->d_name) + strlen(path) + 2);
		strcpy(sFullEntryName, path);
		strcat(sFullEntryName, chr_to_str(c_separator()));
		strcat(sFullEntryName, entry->d_name);
		if ((entry->d_type == DT_DIR) && (isRecursive)){
			if (strcmp(entry->d_name, "..") == 0 || (strcmp(entry->d_name, ".") == 0))
					continue;
			ret = shred_dir(sFullEntryName, isRecursive, isDelete);
		}
		else if (entry->d_type == DT_REG){
			//target entry is a file
			printf("shredding file %s\n", sFullEntryName);
			ret = shred_file(sFullEntryName);
		}
		if (isDelete)
			sremove(sFullEntryName);
		free(sFullEntryName);
    }

    closedir(hStream);
    return ret;
}


void show_help(char* command)
{
	printf( "Desc  : Securely erase your data\n"
				"Author: sandwichdoge@gmail.com\n\n"
				"usage     : %s <file/directory_path> [-r] [-d]\n"
				"parameters:\n\n"
				"-r:	recursive\n"
				"-d:	scramble name and delete target after shredding.\n", command);
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


	//target is a directory
	if (is_directory(sTargetPath)){
		char x;
		int isSuccessful;
		printf("this will shred all files and subfolders within this directory. procceed? y/n: ");
		x = getchar();
		if (x == 'y'){
			isSuccessful = shred_dir(sTargetPath, isRecursive, isDelete);
			(isSuccessful = -1) ? printf("failed to shred some data.\n") :
			printf("operation on directory %s has completed, recursive = %d\n", sTargetPath, isRecursive);
		}
		return;
	}
	else
		return;


	//target is a file
	unsigned int Sz = file_get_size(sTargetPath);
	if (Sz == 0){
		printf("'%s' does not exist.\n", sTargetPath);
		return;
	}

	printf("shredding file %s\n", sTargetPath);
	printf("%u bytes\n", Sz);

	if (shred_file(sTargetPath) == -1)
		printf("failed to shred file %s\n", sTargetPath);
	else{
		if (isDelete == 1){  //'-d' as argument -> rename and delete file
			printf("scramling filename and removing file..\n");
			sremove(sTargetPath);
		}
	}
	printf("finished shredding.\n");
	return;
} 