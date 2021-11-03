#include "object.h"

#include <string.h>

#include "file.h"
#include "mem.h"
#include "texture.h"

#define objectInitWithNameFail() \
	{                            \
		objectFree(obj);         \
		return NULL;             \
	}

object *objectInit(vec3 position) {
	object *obj = memAlloc("object initialisation", sizeof(object));
	if(!obj) return NULL;

	memset(obj, 0, sizeof(object));

	glmc_vec3_copy(position, obj->position);
	glmc_vec3_broadcast(1.0f, obj->scale);
	objectUpdate(obj);

	return obj;
}

object *objectInitWithName(const char *name, vec3 position) {
	object *obj = objectInit(position);
	if(!obj) return NULL;

	const char *extension = ".bin";

	// Model
	char *filename =
		memString("object model filename", "%s%s", name, extension, NULL);
	if(!filename) objectInitWithNameFail();

	obj->mod = modelInit(filename);
	free(filename);
	if(!obj->mod) objectInitWithNameFail();

	// Textures
	obj->textureCount = 1;
	obj->textures =
		memAlloc("object texture list", obj->textureCount * sizeof(GLuint));
	glGenTextures(obj->textureCount, obj->textures);

	filename = memString("object albedo texture filename", "%s-albedo%s", name,
						 extension, NULL);
	if(!filename) objectInitWithNameFail();

	bool result = textureInit(&obj->textures[0], filename);
	free(filename);
	if(!result) objectInitWithNameFail();

	filename = memString("object model dimensions filename", "dimensions-%s%s",
						 name, extension, NULL);
	if(!filename) objectInitWithNameFail();

	// Dimensions
	file *f = fileInit(filename, fm_read);
	free(filename);
	if(!f) objectInitWithNameFail();

	result = fileRead(f, "model object dimensions radius", &obj->radius,
					  sizeof(float));
	if(!result) {
		fileFree(f);
		objectInitWithNameFail();
	}

	result = fileRead(f, "model object dimensions smallest point",
					  obj->smallest, sizeof(vec3));
	if(!result) {
		fileFree(f);
		objectInitWithNameFail();
	}

	result = fileRead(f, "model object dimensions largest point", obj->largest,
					  sizeof(vec3));
	fileFree(f);
	if(!result) objectInitWithNameFail();

	return obj;
}

void objectFree(object *obj) {
	if(obj) {
		if(obj->textures) {
			glDeleteTextures(obj->textureCount, obj->textures);
			free(obj->textures);

			modelFree(obj->mod);

			free(obj);
			obj = NULL;
		}
	}
}

void objectUpdate(object *obj) {
	glmc_mat4_identity(obj->matrix);
	glmc_translate(obj->matrix, obj->position);

	glmc_rotate_x(obj->matrix, glm_rad(obj->orientation[0]), obj->matrix);
	glmc_rotate_y(obj->matrix, glm_rad(-obj->orientation[1]), obj->matrix);
	glmc_rotate_z(obj->matrix, glm_rad(obj->orientation[2]), obj->matrix);

	glmc_scale(obj->matrix, obj->scale);
}

void objectDraw(object *obj) {
	if(obj->textures)
		for(int i = 0; i < obj->textureCount; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, obj->textures[i]);
		}

	if(obj->mod) modelDraw(obj->mod);
}
