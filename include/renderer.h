#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <GLES3/gl31.h>

#define rendererClear() glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

typedef enum { //
	rt_normal,
	rt_postprocess,
	rt_direct
} rendererType;

typedef struct {
	char *name;
	rendererType type;
	int width, height;

	GLuint program;

	int numOutputs;
	GLuint *outputs;
	GLuint fbo;

	int numDrawBuffers;
	GLenum *drawBuffers;
} renderer;

void rendererSystemInit();

renderer *rendererInit(const char *name,
					   rendererType type,
					   int width,
					   int height,
					   int numOutputs);
void rendererFree(renderer *ren);

void rendererResize(renderer *ren, int width, int height);

void rendererUse(renderer *ren);
void rendererComplete(renderer *ren);

#endif // __RENDERER_H__
