#ifndef __MODEL_H__
#define __MODEL_H__

#include <GLES3/gl31.h>

typedef struct {
	GLuint vao, buffers[2];
	GLint count;
} model;

model *modelInit(const char *filename);
void modelFree(model *mod);

void modelDraw(const model *mod);

#endif // __MODEL_H__
