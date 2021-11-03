#include "object.h"

#include <string.h>

#include "mem.h"
#include "texture.h"

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

	char *filename =
		memString("object model filename", "%s%s", name, extension, NULL);
	if(!filename) {
		objectFree(obj);
		return NULL;
	}

	obj->mod = modelInit(filename);
	free(filename);
	if(!obj->mod) {
		objectFree(obj);
		return NULL;
	}

	obj->textureCount = 1;
	obj->textures =
		memAlloc("object texture list", obj->textureCount * sizeof(GLuint));
	glGenTextures(obj->textureCount, obj->textures);

	filename = memString("object albedo texture filename", "%s-albedo%s", name,
						 extension, NULL);
	if(!filename) {
		objectFree(obj);
		return NULL;
	}

	bool result = textureInit(&obj->textures[0], filename);
	free(filename);
	if(!result) {
		objectFree(obj);
		return NULL;
	}

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
	// glUniformMatrix4fv(0, 1, GL_FALSE, obj->matrix[0]);

	if(obj->textures)
		for(int i = 0; i < obj->textureCount; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, obj->textures[i]);
		}

	if(obj->mod) modelDraw(obj->mod);
}
