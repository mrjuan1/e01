#include "mem.h"

#include <malloc.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"

void *memAlloc(const char *thing, int size) {
	void *result = malloc(size);
	if(!result) {
		err("Failed to allocate memory for %s.\n", thing);
		return NULL;
	}

	return result;
}

char *memString(const char *thing, const char *text, ...) {
	va_list list;
	va_start(list, text);

	char *arg = NULL;
	int len = strlen(text) + 1;
	while((arg = va_arg(list, char *))) len += strlen(arg);

	va_end(list);

	char *result = memAlloc(thing, len);
	if(!result) return NULL;
	memset(result, 0, len);

	va_start(list, text);
	vsprintf(result, text, list);
	va_end(list);

	len = strlen(result);
	result = realloc(result, len + 1);
	if(!result) {
		err("Failed to re-allocate memory for string %s.\n", thing);
		return NULL;
	}

	result[len] = '\0';

	return result;
}
