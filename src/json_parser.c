#include <stdio.h>
#include <stdbool.h>
#include <string.h>
//#include <jansson.h>
#include <json-c/json.h>
#include "json_parser.h"


/**
  * This functions returns the property value of the passed in property and json string.
  * It uses the Jansson library to parse the JSON.
  * It also checks whether the json string is an object or array
*/

// jansson version
//
// char const *getPropertyValueFromJson(char *jsonString, char *property) {
// 	printf("C SDK - getPropertyValueFromJson\n");

// 	const char *value = NULL;
// 	json_t *root, *jsonValue;
// 	json_error_t error;

// 	root = json_loads(jsonString, 0, &error);

// 	if (json_is_object(root)) {
// 		jsonValue = json_object_get(root, property);
// 		if (json_is_string(jsonValue)) {
// 			value = json_string_value(jsonValue);
// 		} else if (json_is_object(jsonValue) || json_is_array(jsonValue)) {
// 			value = jsonString;
// 		}
// 		char *rval = NULL;
// 		rval = strdup(value);
// 		if (root->refcount > 0) {
// 			json_decref(root);
// 		}
// 		return rval;
// 	} else {
// 		if (root->refcount > 0) {
// 			json_decref(root);
// 		}
// 		return NULL;
// 	}
// }

// json-c version
//
char const *getPropertyValueFromJson(char *jsonString, char *property) {
	printf("C SDK - getPropertyValueFromJson\n");

	const char *value = NULL;
	json_object *root = json_tokener_parse(jsonString);
	json_object *jsonValue;
 
	if (json_object_get_type(root) == json_type_object) {
		jsonValue = json_object_object_get(root, property);
		if (jsonValue != NULL) {
			if (jsonValue != NULL && json_object_get_type(jsonValue) == json_type_string) {
				value = json_object_get_string(jsonValue);
			} else if (json_object_get_type(jsonValue) == json_type_object || json_object_get_type(jsonValue) == json_type_array) {
				value = jsonString;
			}
			char *rval = NULL;
			rval = strdup(value);
			if (root) {
				json_object_put(root);
			}
			return rval;
			} else {
				return NULL;
			}
	} else {
		if (root) {
			json_object_put(root);
		}
		return NULL;
	}
}

/**
  * Checks if the json string is an array
*/

// jansson version
//
// bool checkIfJsonArray(char *jsonString) {
// 	printf("C SDK - checkIfJsonArray\n");

// 	json_t *root;
// 	json_error_t error;

// 	root = json_loads(jsonString, 0, &error);

// 	if (json_is_array(root)) {
// 		json_decref(root);
// 		return true;
// 	} else {
// 		json_decref(root);
// 		return false;
// 	}
// }

// json-c version
//
bool checkIfJsonArray(char *jsonString) {
	printf("C SDK - checkIfJsonArray\n");

	json_object *root = json_tokener_parse(jsonString);

	if (json_object_get_type(root) == json_type_array) {
		json_object_put(root);
		return true;
	} else {
		json_object_put(root);
		return false;
	}
}