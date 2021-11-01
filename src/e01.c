#include "e01.h"

#include <cglm/call.h>

#include "config.h"
#include "file.h"
#include "log.h"
#include "mem.h"
#include "renderer.h"
#include "window.h"

#if 1
#include <GLES2/gl2ext.h>
#endif // 1

const GLsizei stride = (3 + 2) * sizeof(float);

renderer *base = NULL;
renderer *basic = NULL;

mat4 projection = GLM_MAT4_IDENTITY_INIT;
mat4 view = GLM_MAT4_IDENTITY_INIT;
mat4 projectionView = GLM_MAT4_IDENTITY_INIT;

GLuint vaos[2], buffers[4];
int indexCount = 0;
GLuint texture;

bool resized = false;
float angle = 0.0f;

void updateProjectionView();

bool e01Init() {
	const float shade = 0.15f;
	glClearColor(shade, shade, shade, 1.0f);

	base = rendererInit("base", rt_normal, 0, 0, 0);
	if(!base) return false;

	basic = rendererInit("basic", rt_postprocess, 0, 0, 2);
	if(!basic) return false;

	file *f = fileInit("model.bin", fm_read);
	if(!f) return false;

	int vertexDataSize;
	bool result =
		fileRead(f, "model vertex data size", &vertexDataSize, sizeof(int));
	if(!result) {
		fileFree(f);
		return false;
	}

	void *vertexData = fileReadAlloc(f, "model vertex data", vertexDataSize);
	if(!vertexData) {
		fileFree(f);
		return false;
	}

	int indexDataSize;
	result = fileRead(f, "model index data size", &indexDataSize, sizeof(int));
	if(!result) {
		free(vertexData);
		fileFree(f);

		return false;
	}

	indexCount = indexDataSize / sizeof(unsigned int);

	void *indexData = fileReadAlloc(f, "model index data", indexDataSize);
	fileFree(f);
	if(!indexData) {
		free(vertexData);
		return false;
	}

	rendererUse(base);

	glmc_translate(view, (vec3) { 0.0f, 0.0f, -5.0f });
	updateProjectionView();

	glGenVertexArrays(2, vaos);
	glGenBuffers(4, buffers);

	glBindVertexArray(vaos[0]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);
	free(vertexData);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
						  (const void *)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData,
				 GL_STATIC_DRAW);
	free(indexData);

	glUniform4fv(1, 1, GLM_VEC4_ONE);
	glUniform1i(2, 0);
	glUniform1i(3, 0);

	rendererUse(basic);

	glBindVertexArray(vaos[1]);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	const GLsizeiptr quadDataSize = (2 + 2) * 4 * sizeof(float);
	const float quadData[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, // 0
		1.0f,  -1.0f, 1.0f, 0.0f, // 1
		1.0f,  1.0f,  1.0f, 1.0f, // 2
		-1.0f, 1.0f,  0.0f, 1.0f  // 3
	};
	glBufferData(GL_ARRAY_BUFFER, quadDataSize, (const void *)quadData,
				 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[3]);
	const GLsizeiptr quadIndexSize = 3 * 2 * sizeof(unsigned char);
	const unsigned char quadIndex[] = {
		0, 1, 2, // 0
		0, 2, 3	 // 1
	};
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndexSize,
				 (const void *)quadIndex, GL_STATIC_DRAW);

	glUniform2f(0, (float)windowWidth(), (float)windowHeight());
	glUniform1i(1, configGetSamples());
	glUniform1i(2, 0);
	glUniform1i(3, 1);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	f = fileInit("texture.bin", fm_read);
	fileSeek(f, f->size - sizeof(unsigned char));
	unsigned char levels;
	fileRead(f, "texture levels", &levels, sizeof(unsigned char));
	debug("Levels: %u\n", levels);
	fileSeek(f, 0);
	for(int i = 0; i < levels; i++) {
		unsigned short width, height;
		fileRead(f, "texture level width", &width, sizeof(unsigned short));
		fileRead(f, "texture level height", &height, sizeof(unsigned short));
		debug("Texture level %i size: %ux%u\n", i, width, height);

		int size;
		fileRead(f, "texture level size", &size, sizeof(int));

		char *data = memAlloc("texture data", size);
		fileRead(f, "texture data", data, size);
		glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_COMPRESSED_RGB8_ETC2, width,
							   height, 0, size, data);
		free(data);
	}
	fileFree(f);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLint anisotropy;
	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glUseProgram(base->program);
	glUniform1i(2, 1);

	return true;
}

void e01Free() {
	glDeleteTextures(1, &texture);
	glDeleteBuffers(4, buffers);
	glDeleteVertexArrays(2, vaos);

	rendererFree(basic);
	rendererFree(base);
}

void e01Resize() {
	rendererResize(base, 0, 0);
	rendererResize(basic, 0, 0);

	updateProjectionView();

	resized = true;
}

void e01Run() {
	rendererUse(base);

	const float speed = 25.0f * windowDeltaTime();
	angle += speed;
	if(angle > 360.0f) angle -= 360.0f;

	mat4 matrix;
	glmc_rotate_y(projectionView, glm_rad(-angle), matrix);
	glUniformMatrix4fv(0, 1, GL_FALSE, matrix[0]);

	glBindVertexArray(vaos[0]);
	glBindTexture(GL_TEXTURE_2D, texture);

	rendererClear();

	glCullFace(GL_FRONT);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL);
	glCullFace(GL_BACK);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL);

	rendererComplete(base);

	rendererUse(basic);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindVertexArray(vaos[1]);

	if(resized) {
		glUniform2f(0, (float)windowWidth(), (float)windowHeight());
		resized = false;
	}

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);
}

void updateProjectionView() {
	glmc_perspective(glm_rad(75.0f),
					 (float)windowWidth() / (float)windowHeight(), 1.0f, 10.0f,
					 projection);
	glmc_mul(projection, view, projectionView);
}
