#include "model.h"

#include <malloc.h>
#include <string.h>

#include "file.h"
#include "mem.h"

const GLsizei modelStride = (3 + 2) * sizeof(float);

model *modelInit(const char *filename) {
	file *f = fileInit(filename, fm_read);
	if(!f) return NULL;

	int vertexDataSize;
	bool result =
		fileRead(f, "model vertex data size", &vertexDataSize, sizeof(int));
	if(!result) {
		fileFree(f);
		return NULL;
	}

	void *vertexData = fileReadAlloc(f, "model vertex data", vertexDataSize);
	if(!vertexData) {
		fileFree(f);
		return NULL;
	}

	int indexDataSize;
	result = fileRead(f, "model index data size", &indexDataSize, sizeof(int));
	if(!result) {
		free(vertexData);
		fileFree(f);

		return NULL;
	}

	int indexCount = indexDataSize / sizeof(unsigned int);

	void *indexData = fileReadAlloc(f, "model index data", indexDataSize);
	fileFree(f);
	if(!indexData) {
		free(vertexData);
		return NULL;
	}

	const char *thingString = "model initialisation";
	char *thing = memString(thingString, "model \"%s\"", filename, NULL);
	if(!thing) {
		free(indexData);
		free(vertexData);

		return NULL;
	}

	model *mod = memAlloc(thing, sizeof(model));
	free(thing);
	if(!mod) {
		free(indexData);
		free(vertexData);

		return NULL;
	}

	memset(mod, 0, sizeof(model));

	glGenVertexArrays(1, &mod->vao);
	glGenBuffers(2, mod->buffers);

	glBindVertexArray(mod->vao);

	glBindBuffer(GL_ARRAY_BUFFER, mod->buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);
	free(vertexData);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, modelStride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, modelStride,
						  (const void *)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod->buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData,
				 GL_STATIC_DRAW);
	free(indexData);

	mod->count = indexCount;

	return mod;
}

void modelFree(model *mod) {
	if(mod) {
		glDeleteBuffers(2, mod->buffers);
		glDeleteVertexArrays(1, &mod->vao);

		free(mod);
		mod = NULL;
	}
}

void modelDraw(const model *mod) {
	glBindVertexArray(mod->vao);
	glDrawElements(GL_TRIANGLES, mod->count, GL_UNSIGNED_INT, NULL);
}
