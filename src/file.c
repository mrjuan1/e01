#include "file.h"

#include <SDL2/SDL_rwops.h>

#include "log.h"
#include "mem.h"

#define fileInitFail()    \
	{                     \
		fileFree(result); \
		return NULL;      \
	}

file *fileInit(const char *filename, fileMode mode) {
	char *thing = memString("file loading", "file \"%s\"", filename, NULL);
	if(!thing) return NULL;

	file *result = memAlloc(thing, sizeof(file));
	free(thing);
	if(!result) return NULL;

	memset(result, 0, sizeof(file));

	SDL_RWops *f = SDL_RWFromFile(filename, mode == fm_read ? "rb" : "wb");
	if(!f) {
		err("%s\nFailed to open file \"%s\" for %s.\n", SDL_GetError(),
			filename, mode == fm_read ? "reading" : "writing");
		fileInitFail();
	}
	result->ptr = f;

	result->name = strdup(filename);
	if(!result->name) {
		err("Failed to store file name for file \"%s\".\n", filename);
		fileInitFail();
	}

	result->size = SDL_RWsize(f);
	if(result->size < 0) {
		err("%s\nFailed to get size for file \"%s\".\n", SDL_GetError(),
			filename);
		fileInitFail();
	}

	return result;
}

void fileFree(file *f) {
	if(f) {
		if(f->ptr)
			if(SDL_RWclose((SDL_RWops *)f->ptr)) {
				const char *msg = "Failed to close file";

				if(f->name)
					warn("%s\n%s \"%s\".\n", SDL_GetError(), msg, f->name);
				else
					warn("%s\n%s.\n", SDL_GetError(), msg);
			}

		if(f->name) free(f->name);

		free(f);
		f = NULL;
	}
}

bool fileRead(file *f, const char *thing, void *data, int size) {
	int result = SDL_RWread((SDL_RWops *)f->ptr, data, size, 1);
	if(result != 1) {
		err("%s\nFailed to read %s from file \"%s\".\n", SDL_GetError(), thing,
			f->name);
		return false;
	}

	return true;
}

bool fileWrite(file *f, const char *thing, const void *data, int size) {
	int result = SDL_RWwrite((SDL_RWops *)f->ptr, data, size, 1);
	if(result != 1) {
		err("%s\nFailed to write %s to file \"%s\".\n", SDL_GetError(), thing,
			f->name);
		return false;
	}

	return true;
}

void *fileReadAlloc(file *f, const char *thing, int size) {
	void *result = memAlloc(thing, size);
	if(!result) return NULL;

	if(!fileRead(f, thing, result, size)) {
		free(result);
		return NULL;
	}

	return result;
}

void *fileReadAllocAll(const char *filename, int *size) {
	file *f = fileInit(filename, fm_read);
	if(!f) return NULL;

	void *result = fileReadAlloc(f, "file data", f->size);
	if(!result) {
		fileFree(f);
		return NULL;
	}

	if(size) *size = f->size;

	return result;
}
