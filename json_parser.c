#include <stdio.h>
#include <jansson.h>
#include "json_parser.h"

char const *getPropertyValueFromJson(char *jsonString, char *property) {
	json_t *root, *jsonValue;
	json_error_t error;

	root = json_loads(jsonString, 0, &error);

	if (json_is_object(root)) {
		jsonValue = json_object_get(root, property);
		const char *value = json_string_value(jsonValue);
		return value;
	} else {
		return NULL;
	}
}