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


void generate_random_data(char *buffer, const int data_len){
	//TODO: better randomization, possibly more patterns
	unsigned char c;
	int i;
	for(i = 0; i<data_len; ++i){
		c = rand() % (253 + 1 - 1) + 1;
		*(buffer+i) = c;
	}

}


void repeat_char(char *buffer, const char c, int count, const int is_raw_data)
{
	memset(buffer, c, count);
	if (is_raw_data == 0) buffer[count] = '\0';//return raw null-terminated string

}


void sremove(char* target_path)
{
	//scramble file/dir's name before deleting it.
	char* old_filedir = extract_file_dir(target_path, 1);
	char new_filename[256];
	repeat_char(new_filename, 'a', (int)(strlen(extract_file_name(target_path))), 0);
	char* new_filepath = str_cat(old_filedir, new_filename);
	rename(target_path, new_filepath);
	if (is_directory(new_filepath))
		rmdir(new_filepath);
	else
		remove(new_filepath);
	
	free(new_filepath);
	free(old_filedir);

}


int shred_file(char* target_path)
{
	int buffer_sz;
	char buffer[1024];
	unsigned int sz = file_get_size(target_path);
	if (sz == 0)
		return -1;
	unsigned int sz_kB = sz/1024 + (sz%1024!=0);  //round up, number of buffers needed

	for (int pass = 1; pass <= 2; ++pass)
	{
		int fp = open(target_path, O_WRONLY);
		if (sz >= 1024)
		{
			for(int i = 1; i<=sz_kB; ++i)  //write 1024bytes at a time
			{
				if (i==sz_kB)
					buffer_sz = sz % 1024;  //exact size of last buffer
				else
					buffer_sz = 1024;
				if     (pass == 1)       //1st pass = zero data
					repeat_char(buffer, '\0', buffer_sz, 1);
				else if(pass == 2)  //2nd pass = write random data
					generate_random_data(buffer, buffer_sz);

				write(fp, buffer, buffer_sz);
			}
		}
		else
		{
			generate_random_data(buffer, sz);
			write(fp, buffer, sz);
		}
		close(fp);
	}
	return 0;
}

int shred_dir(char* path, int is_recursive, int is_remove)
{
    //recursive
    DIR *fd_stream ;
    if (!(fd_stream = opendir(path))) {
		printf("cannot access %s\n", path);
        return -1;
	}
    struct dirent *entry;
    char* full_entry_name;
	int ret;
    while ((entry = readdir(fd_stream))){
		full_entry_name = malloc(strlen(entry->d_name) + strlen(path) + 2);
		strcpy(full_entry_name, path);
		strcat(full_entry_name, chr_to_str(c_separator()));
		strcat(full_entry_name, entry->d_name);
		if ((entry->d_type == DT_DIR) && (is_recursive)){
			if (strcmp(entry->d_name, "..") == 0 || (strcmp(entry->d_name, ".") == 0))
					continue;
			ret = shred_dir(full_entry_name, is_recursive, is_remove);
		}
		else if (entry->d_type == DT_REG){
			//target entry is a file
			printf("shredding file %s\n", full_entry_name);
			ret = shred_file(full_entry_name);
		}
		if (is_remove)
			sremove(full_entry_name);
		free(full_entry_name);
    }

    closedir(fd_stream);
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

	char* target_path = argv[1];
	int is_recursive = 0;
	int is_remove = 0;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp("-r", argv[i]) == 0)
			is_recursive = 1;
		if (strcmp("-d", argv[i]) == 0)
			is_remove = 1;
	}

	srand(time(NULL));

	//target is a directory
	if (is_directory(target_path)){
		char x;
		int isSuccessful;
		printf("this will shred all files and subfolders within this directory. procceed? y/n: ");
		x = getchar();
		if (x == 'y'){
			isSuccessful = shred_dir(target_path, is_recursive, is_remove);
			(isSuccessful == -1) ? printf("failed to shred some data.\n") :
			printf("operation on directory %s has completed, recursive = %d\n", target_path, is_recursive);
		}
		return;
	}
	else
		return;


	//target is a file
	unsigned int sz = file_get_size(target_path);
	if (sz == 0){
		printf("'%s' does not exist.\n", target_path);
		return;
	}

	printf("shredding file %s\n", target_path);
	printf("%u bytes\n", sz);

	if (shred_file(target_path) == -1)
		printf("failed to shred file %s\n", target_path);
	else{
		if (is_remove == 1){  //'-d' as argument -> rename and delete file
			printf("scramling filename and removing file..\n");
			sremove(target_path);
		}
	}
	printf("finished shredding.\n");
	return;
} 