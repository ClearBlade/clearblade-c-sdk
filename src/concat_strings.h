#ifndef _concat_strings_h
#define _concat_strings_h


#ifdef __cplusplus
extern "C" {
#endif

/** 
  * A function to concat two strings. User needs to free the finalString after use to avoid memory leaks
*/
char *getConcatString(char *str1, char *str2);

#ifdef __cplusplus
}
#endif

#endif