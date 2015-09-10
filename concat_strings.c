#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "concat_strings.h"

/** 
  * A function to concat two strings. User needs to free the finalString after use to avoid memory leaks
*/
char *getConcatString(char *str1, char *str2) {
	char *finalString = malloc(1 + strlen(str1) + strlen(str2)); // Needs to be freed by the user after use
	assert(finalString != NULL);
	strcpy(finalString, str1);
	strcat(finalString, str2);

	return finalString;
}
