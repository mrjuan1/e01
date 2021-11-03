#include "e01.h"

#include <cglm/call.h>

#include "config.h"
#include "model.h"
#include "renderer.h"
#include "texture.h"
#include "window.h"

renderer *base = NULL;
renderer *basic = NULL;

mat4 projection = GLM_MAT4_IDENTITY_INIT;
mat4 view = GLM_MAT4_IDENTITY_INIT;
mat4 projectionView = GLM_MAT4_IDENTITY_INIT;

model *quad = NULL, *camera = NULL;
GLuint texture = 0;

bool resized = false;
float angle = 0.0f;

void updateProjectionView();

bool e01Init() {
	base = rendererInit("base", rt_normal, 0, 0, 0);
	if(!base) return false;

	basic = rendererInit("basic", rt_direct, 0, 0, 2);
	if(!basic) return false;

	quad = modelInit("quad.bin");
	if(!quad) return false;

	camera = modelInit("camera.bin");
	if(!camera) return false;

	if(!textureInit(&texture, "texture.bin")) return false;

	const float shade = 0.1f;
	glClearColor(shade, shade, shade, 1.0f);

	glmc_translate(view, (vec3) { 0.0f, 0.0f, -5.0f });
	updateProjectionView();

	camera->hasTransparency = true;

	rendererUse(base);
	glUniform4fv(1, 1, GLM_VEC4_ONE);
	glUniform1i(2, 1);
	glUniform1i(3, 0);

	rendererUse(basic);
	glUniform2f(0, (float)windowWidth(), (float)windowHeight());
	glUniform1i(1, configGetSamples());
	glUniform1i(2, 0);
	glUniform1i(3, 1);

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
	// Base pass
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

	// Basic pass
	rendererUse(basic);

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
