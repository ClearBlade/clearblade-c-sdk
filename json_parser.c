#include <stdio.h>
#include <jansson.h>
#include "json_parser.h"

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