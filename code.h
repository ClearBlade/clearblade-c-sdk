#ifndef _code_h
#define _code_h


/**
  * Function to execute code service without parameters. Service name and codeCallback are required parameters
*/
void executeCodeServiceWithoutParams(char *serviceName, void (*codeCallback)(bool error, char *result));


/**
  * Function to execute code service with parameters. Service name, params and codeCallback are required parameters
  * Params need to be passes as a json string
*/
void executeCodeServiceWithParams(char *serviceName, char *params, void (*codeCallback)(bool error, char *result));

#endif