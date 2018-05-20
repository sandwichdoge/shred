#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//All values returned from these functions need to be freed after it's used.

char* str_right(char* string, const size_t count)
{
	char* ret = malloc(count * sizeof(char));
	ret = string;
	ret = ret + strlen(string) - count;
	return ret;
}


char* str_left(const char* string, const size_t count)
{
	char* ret = malloc(count * sizeof(char) + 1);
	memmove(ret, string, count);
	*(ret + count) = '\0';
	return ret;
}


char* str_mid(char* string, const size_t pos, const size_t count)
{
	char* ret = malloc(count * sizeof(char) + 1);
	char* tmp = string;
	tmp += pos;
	memmove(ret, tmp, count);
	*(ret+count) = '\0';
	return ret;
}


char *chr_repeat(const char _c, const size_t count)
{
	char *ret = malloc(count * sizeof(char) + 1);
	memset(ret, _c, count);
	*(ret + count) = '\0';
	return ret;
}


char *str_repeat(char* string, const size_t count)
{
	char *ret = malloc(count * strlen(string) + 1);
	for (int i = 1; i < count; ++i)
		strcat(ret, string);
	*(ret+count*strlen(string)) = '\0';
	return ret;
}


int str_count(char* string, const char* substr)
{
	int count = 0;
	size_t substrSz = strlen(substr);
	char *tmp;
	for (int i = 0; i < strlen(string); ++i)
	{
		tmp = str_mid(string, i, substrSz);
		if (strcmp(tmp ,substr) == 0)
			++count;
	}
	free(tmp);
	return count;
}


char* str_cat(char* str1, char* str2)
{
	char *ret = malloc(strlen(str1) + strlen(str2)+ 1);
	char *tmp = ret;
	memcpy(tmp, str1, strlen(str1));
	tmp += strlen(str1);
	memcpy(tmp, str2, strlen(str2));
	*(tmp+strlen(str2)) = '\0';
	return ret;
}

char* chr_to_str(const char _c)
{
	char* ret = malloc(2);
	*(ret) = _c;
	*(ret+1) = '\0';
	return ret;
}

char** str_split(char* string, const char* delim, size_t *refOccurs)
{
	//reinventing the wheel..could easy go with progressive string traverse method.
	size_t delimSz = strlen(delim);
	int curpos = 0;
	size_t index = 0;
	size_t occurrences = str_count(string, delim);
	*refOccurs = occurrences + 1;
	char* substr;
	char** tmp = malloc(occurrences * sizeof(char*));

	for (int i = 0; i < strlen(string); ++i)
	{
		substr = str_mid(string, i, delimSz);
		if (strcmp(substr , delim) == 0 || index == occurrences){
			if (index == occurrences)  //the rest of the string
				i = strlen(string);
			tmp[index] = (char*)malloc(i - curpos + 1);
			memcpy(tmp[index], str_mid(string, curpos, i - curpos), i - curpos);
			*(tmp[index] + i - curpos) = '\0';
			++index;
			curpos = i + delimSz;
		}
		
	}
	return tmp;
}
