#include <stdio.h>
#include <stdbool.h>
#include <jansson.h>
#include "json_parser.h"


/**
  * This functions returns the property value of the passed in property and json string.
  * It uses the Jansson library to parse the JSON.
  * It also checks whether the json string is an object or array
*/
char const *getPropertyValueFromJson(char *jsonString, char *property) {
	const char *value = NULL;
	json_t *root, *jsonValue;
	json_error_t error;

	root = json_loads(jsonString, 0, &error);

	if (json_is_object(root)) {
		jsonValue = json_object_get(root, property);
		if (json_is_string(jsonValue)) {
			value = json_string_value(jsonValue);
		} else if (json_is_object(jsonValue) || json_is_array(jsonValue)) {
			value = jsonString;
		}
		return value;
	} else {
		return NULL;
	}
}

/**
  * Checks if the json string is an array
*/
bool checkIfJsonArray(char *jsonString) {
	json_t *root;
	json_error_t error;

	root = json_loads(jsonString, 0, &error);

	if (json_is_array(root)) {
		return true;
	} else {
		return false;
	}
}