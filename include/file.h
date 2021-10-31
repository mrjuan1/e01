#ifndef __FILE_H__
#define __FILE_H__

#include <stdbool.h>

typedef struct {
	void *ptr;
	char *name;
	int size;
} file;

typedef enum { //
	fm_read,
	fm_write
} fileMode;

file *fileInit(const char *filename, fileMode mode);
void fileFree(file *f);

bool fileRead(file *f, const char *thing, void *data, int size);
bool fileWrite(file *f, const char *thing, const void *data, int size);

void *fileReadAlloc(file *f, const char *thing, int size);
void *fileReadAllocAll(const char *filename, int *size);

#endif // __FILE_H__
