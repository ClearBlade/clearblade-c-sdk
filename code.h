#ifndef _code_h
#define _code_h

void executeCodeServiceWithoutParams(char *serviceName, void (*codeCallback)(bool error, char *result));
void executeCodeServiceWithParams(char *serviceName, char *params, void (*codeCallback)(bool error, char *result));

#endif