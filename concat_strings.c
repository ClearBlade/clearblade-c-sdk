#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "concat_strings.h"

char *getConcatString(char *str1, char *str2) {
	char *finalString = malloc(1 + strlen(str1) + strlen(str2));
	strcpy(finalString, str1);
	strcat(finalString, str2);

	return finalString;
}
