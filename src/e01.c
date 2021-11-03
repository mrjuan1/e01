#include "e01.h"

#include <cglm/call.h>

#include "config.h"
#include "file.h"
#include "log.h"
#include "mem.h"
#include "model.h"
#include "renderer.h"
#include "window.h"

#if 1
#include <GLES2/gl2ext.h>
#endif // 1

renderer *base = NULL;
renderer *basic = NULL;

mat4 projection = GLM_MAT4_IDENTITY_INIT;
mat4 view = GLM_MAT4_IDENTITY_INIT;
mat4 projectionView = GLM_MAT4_IDENTITY_INIT;

model *quad = NULL, *camera = NULL;
GLuint texture;

bool resized = false;
float angle = 0.0f;

void updateProjectionView();

bool e01Init() {
	const float shade = 0.1f;
	glClearColor(shade, shade, shade, 1.0f);

	base = rendererInit("base", rt_normal, 0, 0, 0);
	if(!base) return false;

	basic = rendererInit("basic", rt_postprocess, 0, 0, 2);
	if(!basic) return false;

	quad = modelInit("quad.bin");
	if(!quad) return false;

	camera = modelInit("camera.bin");
	if(!camera) return false;

	camera->hasTransparency = true;

	rendererUse(base);

	glmc_translate(view, (vec3) { 0.0f, 0.0f, -5.0f });
	updateProjectionView();

	glUniform4fv(1, 1, GLM_VEC4_ONE);
	glUniform1i(2, 1);
	glUniform1i(3, 0);

	rendererUse(basic);

	glUniform2f(0, (float)windowWidth(), (float)windowHeight());
	glUniform1i(1, configGetSamples());
	glUniform1i(2, 0);
	glUniform1i(3, 1);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	file *f = fileInit("texture.bin", fm_read);
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

	return true;
}

void e01Free() {
	glDeleteTextures(1, &texture);

	modelFree(camera);
	modelFree(quad);

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

	rendererClear();
	glBindTexture(GL_TEXTURE_2D, texture);
	modelDraw(camera);

	rendererComplete(base);

	rendererUse(basic);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(resized) {
		glUniform2f(0, (float)windowWidth(), (float)windowHeight());
		resized = false;
	}

	modelDraw(quad);
}

void updateProjectionView() {
	glmc_perspective(glm_rad(75.0f),
					 (float)windowWidth() / (float)windowHeight(), 1.0f, 10.0f,
					 projection);
	glmc_mul(projection, view, projectionView);
}
