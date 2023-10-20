#ifndef _json_parser_h
#define _json_parser_h

/**
  * This functions returns the property value of the passed in property and json string.
  * It uses the Jansson library to parse the JSON.
  * It also checks whether the json string is an object or array
*/
char const *getPropertyValueFromJson(char *jsonString, char *property);



/**
  * Checks if the json string is an array
*/
bool checkIfJsonArray(char *jsonString);

#endif