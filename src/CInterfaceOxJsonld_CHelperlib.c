#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* copy2cstring(const unsigned char* input){
	if (input == NULL) return NULL;
	char* ret = malloc(strlen(input) + 1);
	strcpy(ret, input);
	return ret;
}
