#ifndef __MODEL_H__
#define __MODEL_H__

#include <GLES3/gl31.h>
#include <stdbool.h>

typedef struct {
	GLuint vao, buffers[2];
	GLint count;

	bool hasTransparency;
} model;

model *modelInit(const char *filename);
void modelFree(model *mod);

void modelDraw(const model *mod);

#endif // __MODEL_H__
