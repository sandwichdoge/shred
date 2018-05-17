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
#include <sys/types.h>
#include <sys/stat.h>


char* generate_random_data(int nDataLen){
	srand(time(NULL));
	char _c;
	char* tmp = malloc(nDataLen * sizeof(char)); //add 1 for \0 if i want a string
	char* ret;
	ret = tmp;
	for(int i = 0; i<nDataLen; ++i){
		_c = rand() % (126 + 1 - 1) + 1;
		*(tmp+i) = _c;
	}
	//*(tmp+nDataLen) = '\0';  //not necessary but jic i want random string sized $nDataLen
	//printf("buf len %lu\n", strlen(ret));

	return ret;
}


char* repeat_char(char _c, int nCount, int isRawData)
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
				if(nPass == 1)       //1st pass = zero data
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

void main(int argc, char **argv){
	
	if (argc != 2)
	{
		printf("usage: %s <file_path>\n", argv[0]);
		return;
	}
	char* sTargetPath = argv[1];
	
	size_t Sz = file_get_size(sTargetPath);
	printf("filename %s -", sTargetPath);
	printf("%zu bytes\n", Sz);
	printf("shredding in progress..\n");
	if (shred_file(sTargetPath) == -1)
		printf("failed to shred file %s\n", sTargetPath);
	else
		printf("finished shredding.\n");

	return;
}
