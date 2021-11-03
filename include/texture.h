#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GLES3/gl31.h>
#include <stdbool.h>

void textureSystemInit();
bool textureInit(GLuint *texture, const char *filename);

#endif // __TEXTURE_H__
