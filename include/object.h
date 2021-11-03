#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <cglm/call.h>

#include "model.h"

typedef struct {
	model *mod;

	int textureCount;
	GLuint *textures;

	vec3 position, orientation, scale;
	mat4 matrix;

	float radius;
	vec3 smallest, largest;
} object;

object *objectInit(vec3 position);
object *objectInitWithName(const char *name, vec3 position);

void objectFree(object *obj);

void objectUpdate(object *obj);
void objectDraw(object *obj);

#endif // __OBJECT_H__
