#include "renderer.h"

#include <GLES3/gl31.h>
#include <malloc.h>
#include <string.h>

#include "config.h"
#include "file.h"
#include "log.h"
#include "mem.h"
#include "window.h"

#define rendererInitFail() \
	{                      \
		rendererFree(ren); \
		return NULL;       \
	}
#define rendererLoadProgramFail()          \
	{                                      \
		fileFree(f);                       \
		return rendererCreateProgram(ren); \
	}
#define rendererSaveProgramFail() \
	{                             \
		fileFree(f);              \
		return false;             \
	}

GLuint rendererLoadShader(const char *name, GLenum type);
bool rendererCreateProgram(renderer *ren);
bool rendererLoadProgram(renderer *ren);
bool rendererSaveProgram(renderer *ren);

void rendererSystemInit() {
	// debug("GL info:\n  Vendor: %s\n  Version: %s\n", glGetString(GL_VENDOR),
	// 	  glGetString(GL_VERSION));

	const int width = windowWidth(), height = windowHeight();
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
	glEnable(GL_SCISSOR_TEST);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepthf(1.0f);
}

renderer *rendererInit(const char *name,
					   rendererType type,
					   int width,
					   int height,
					   int numOutputs) {
	const char *thingString = "renderer initialisation";

	char *thing = memString(thingString, "renderer \"%s\"", name, NULL);
	if(!thing) return NULL;

	renderer *ren = memAlloc(thing, sizeof(renderer));
	free(thing);
	if(!ren) return NULL;

	memset(ren, 0, sizeof(renderer));

	ren->name = strdup(name);
	if(!ren->name) {
		err("Failed to store name for renderer \"%s\".\n", name);
		rendererInitFail();
	}

	ren->type = type;

	if(!rendererLoadProgram(ren)) rendererInitFail();

	if(type == rt_direct) {
		rendererResize(ren, 0, 0);
		return ren;
	}

	if(type == rt_normal && numOutputs < 2)
		numOutputs = 2;
	else if(numOutputs < 1)
		numOutputs = 1;
	ren->numOutputs = numOutputs;

	thing = memString(thingString, "renderer \"%s\" outputs", name, NULL);
	if(!thing) rendererInitFail();

	ren->outputs = memAlloc(thing, numOutputs * sizeof(GLuint));
	free(thing);
	if(!ren->outputs) rendererInitFail();

	glGenFramebuffers(1, &ren->fbo);

	ren->numDrawBuffers = type == rt_normal ? numOutputs - 1 : numOutputs;

	thing = memString(thingString, "renderer \"%s\" draw buffers", name, NULL);
	if(!thing) rendererInitFail();

	ren->drawBuffers = memAlloc(thing, ren->numDrawBuffers * sizeof(GLenum));
	free(thing);
	if(!ren->drawBuffers) rendererInitFail();

	rendererResize(ren, width, height);

	return ren;
}

void rendererFree(renderer *ren) {
	if(ren) {
		if(ren->drawBuffers) free(ren->drawBuffers);
		if(ren->fbo) glDeleteFramebuffers(1, &ren->fbo);

		if(ren->outputs) {
			glDeleteTextures(ren->numOutputs, ren->outputs);
			free(ren->outputs);
		}

		if(ren->program) glDeleteProgram(ren->program);
		if(ren->name) free(ren->name);

		free(ren);
		ren = NULL;
	}
}

void rendererResize(renderer *ren, int width, int height) {
	if(ren->type == rt_direct) {
		ren->width = windowWidth();
		ren->height = windowHeight();

		return;
	}

	if(!width) width = windowWidth();
	if(!height) height = windowHeight();

	// debug("Resizing renderer \"%s\" to %ix%i...\n", ren->name, width,
	// height);

	ren->width = width;
	ren->height = height;

	glDeleteTextures(ren->numOutputs, ren->outputs);
	glGenTextures(ren->numOutputs, ren->outputs);

	glBindFramebuffer(GL_FRAMEBUFFER, ren->fbo);

	const unsigned char samples = configGetSamples();
	GLenum attachment = GL_COLOR_ATTACHMENT0;
	int drawBufferIndex = 0;

	for(int i = 0; i < ren->numOutputs; i++)
		if(ren->type == rt_normal) {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ren->outputs[i]);

			if(i == 1) {
				glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,
										  GL_DEPTH_COMPONENT24, width, height,
										  GL_FALSE);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
									   GL_TEXTURE_2D_MULTISAMPLE,
									   ren->outputs[i], 0);
			} else {
				glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,
										  GL_RGB8, width, height, GL_FALSE);

				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
									   GL_TEXTURE_2D_MULTISAMPLE,
									   ren->outputs[i], 0);

				ren->drawBuffers[drawBufferIndex++] = attachment++;
			}
		} else {
			glBindTexture(GL_TEXTURE_2D, ren->outputs[i]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D,
								   ren->outputs[i], 0);

			ren->drawBuffers[drawBufferIndex++] = attachment++;
		}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void rendererUse(renderer *ren) {
	if(ren->type == rt_normal) {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}

	glViewport(0, 0, ren->width, ren->height);
	glScissor(0, 0, ren->width, ren->height);

	glUseProgram(ren->program);

	if(ren->type == rt_direct) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, ren->fbo);
	glDrawBuffers(ren->numDrawBuffers, ren->drawBuffers);
}

void rendererComplete(renderer *ren) {
	if(ren->type == rt_direct) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	for(int i = 0; i < ren->numOutputs; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(
			ren->type == rt_normal ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
			ren->outputs[i]);
	}
}

GLuint rendererLoadShader(const char *name, GLenum type) {
	const char *thingString = "shader loading";

	char *thing = memString(thingString, "shader filename for renderer \"%s\"",
							name, NULL);
	if(!thing) return 0;

	char *filename =
		memString(thing, "shaders/%s.%s", name,
				  type == GL_VERTEX_SHADER ? "vert" : "frag", NULL);
	free(thing);
	if(!filename) return 0;

	int size;
	char *data = fileReadAllocAll(filename, &size);
	free(filename);
	if(!data) return 0;

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar **)&data, &size);
	free(data);

	glCompileShader(shader);

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
	if(size > 0) {
		thing = memString(thingString, "shader info log for renderer \"%s\"",
						  name, NULL);

		if(thing) {
			data = memAlloc(thing, size);
			free(thing);

			if(data) {
				glGetShaderInfoLog(shader, size, NULL, data);
				info("%s shader for program \"%s\" info:\n%s\n",
					 type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", name,
					 data);
				free(data);
			}
		}
	}

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(!result)
		err("Failed to compile %s shader for program \"%s\".\n",
			type == GL_VERTEX_SHADER ? "vertex" : "fragment", name);

	return shader;
}

bool rendererCreateProgram(renderer *ren) {
	// debug("Creating program for renderer \"%s\"...\n", ren->name);

	ren->program = glCreateProgram();

	GLuint vertexShader = rendererLoadShader(ren->name, GL_VERTEX_SHADER);
	GLuint fragmentShader = rendererLoadShader(ren->name, GL_FRAGMENT_SHADER);

	glAttachShader(ren->program, vertexShader);
	glAttachShader(ren->program, fragmentShader);
	glLinkProgram(ren->program);
	glAttachShader(ren->program, fragmentShader);
	glAttachShader(ren->program, vertexShader);

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	GLint result;
	glGetProgramiv(ren->program, GL_INFO_LOG_LENGTH, &result);
	if(result > 0) {
		char *thing =
			memString("program creation",
					  "program info log for renderer \"%s\"", ren->name, NULL);

		if(thing) {
			GLchar *infoLog = memAlloc(thing, result);
			free(thing);

			if(infoLog) {
				glGetProgramInfoLog(ren->program, result, NULL, infoLog);
				info("Program \"%s\" info:\n%s\n", ren->name, infoLog);
				free(infoLog);
			}
		}
	}

	glGetProgramiv(ren->program, GL_LINK_STATUS, &result);
	if(!result) {
		err("Failed to link program \"%s\".\n", ren->name);
		return false;
	}

	glUseProgram(ren->program);

	return rendererSaveProgram(ren);
}

bool rendererLoadProgram(renderer *ren) {
	const char *thingString = "program binary loading";

	const char *programBinary = "program binary";
	const char *forRenderer = "for renderer";

	// debug("Loading %s %s \"%s\"...\n", programBinary, forRenderer,
	// ren->name);

	char *thing = memString(thingString, "filename for %s %s \"%s\"",
							programBinary, forRenderer, ren->name, NULL);
	if(!thing) return false;

	char *filename = memString(thing, "%s.bin", ren->name, NULL);
	free(thing);
	if(!filename) return false;

	file *f = fileInit(filename, fm_read);
	free(filename);
	if(!f) {
		info(
			"Program binary for renderer \"%s\" could not be loaded, attempting to create it...\n",
			ren->name);
		return rendererCreateProgram(ren);
	}

	thing = memString(thingString, "%s identifier length %s \"%s\"",
					  programBinary, forRenderer, ren->name, NULL);
	if(!thing) rendererLoadProgramFail();

	unsigned char identifierLength;
	bool result = fileRead(f, thing, &identifierLength, sizeof(unsigned char));
	free(thing);
	if(!result) rendererLoadProgramFail();

	char *identifierThing =
		memString(thingString, "%s identifier %s \"%s\"", programBinary,
				  forRenderer, ren->name, NULL);
	if(!identifierThing) rendererLoadProgramFail();

	char *identifier = memAlloc(identifierThing, identifierLength);
	if(!identifier) {
		free(identifierThing);
		rendererLoadProgramFail();
	}

	result = fileRead(f, identifierThing, identifier, identifierLength);
	if(!result) {
		free(identifier);
		free(identifierThing);

		rendererLoadProgramFail();
	}

	char *localIdentifier =
		memString(identifierThing, "%s %s", glGetString(GL_VENDOR),
				  glGetString(GL_VERSION), NULL);
	free(identifierThing);
	if(!localIdentifier) {
		free(identifier);
		rendererLoadProgramFail();
	}

	result = !strcmp(identifier, localIdentifier);
	free(localIdentifier);
	free(identifier);
	if(!result) {
		err("Program binary identifier for renderer \"%s\" does not match what is stored in the program binary file, attempting to re-create the program...\n",
			ren->name);
		rendererLoadProgramFail();
	}

	thing = memString(thingString, "%s format %s \"%s\"", programBinary,
					  forRenderer, ren->name, NULL);
	if(!thing) rendererLoadProgramFail();

	GLenum format;
	result = fileRead(f, thing, &format, sizeof(GLenum));
	free(thing);
	if(!result) rendererLoadProgramFail();

	thing = memString(thingString, "%s size %s \"%s\"", programBinary,
					  forRenderer, ren->name, NULL);
	if(!thing) rendererLoadProgramFail();

	GLint size;
	result = fileRead(f, thing, &size, sizeof(GLint));
	free(thing);
	if(!result) rendererLoadProgramFail();

	thing = memString(thingString, "%s data %s \"%s\"", programBinary,
					  forRenderer, ren->name, NULL);
	if(!thing) rendererLoadProgramFail();

	void *data = fileReadAlloc(f, thing, size);
	free(thing);
	fileFree(f);
	if(!data) return false;

	ren->program = glCreateProgram();
	glProgramBinary(ren->program, format, data, size);
	free(data);

	glValidateProgram(ren->program);

	GLint status;
	glGetProgramiv(ren->program, GL_VALIDATE_STATUS, &status);
	if(!status) {
		err("Program for renderer \"%s\" invalid.\n", ren->name);
		return false;
	}

	glUseProgram(ren->program);

	return true;
}

bool rendererSaveProgram(renderer *ren) {
	const char *thingString = "program binary saving";

	const char *programBinary = "program binary";
	const char *forRenderer = "for renderer";

	debug("Saving %s %s \"%s\"...\n", programBinary, forRenderer, ren->name);

	char *thing = memString(thingString, "filename for %s %s \"%s\"",
							programBinary, forRenderer, ren->name, NULL);
	if(!thing) return false;

	char *filename = memString(thing, "%s.bin", ren->name, NULL);
	free(thing);
	if(!filename) return false;

	file *f = fileInit(filename, fm_write);
	free(filename);
	if(!f) return false;

	char *identifierThing =
		memString(thingString, "%s identifier %s \"%s\"", programBinary,
				  forRenderer, ren->name, NULL);
	if(!identifierThing) rendererSaveProgramFail();

	char *identifier =
		memString(identifierThing, "%s %s", glGetString(GL_VENDOR),
				  glGetString(GL_VERSION), NULL);
	if(!identifier) {
		free(identifierThing);
		rendererSaveProgramFail();
	}

	thing = memString(thingString, "%s identifier length %s \"%s\"",
					  programBinary, forRenderer, ren->name, NULL);
	if(!thing) {
		free(identifier);
		free(identifierThing);

		rendererSaveProgramFail();
	}

	const unsigned char identifierLength = strlen(identifier) + 1;
	bool result = fileWrite(f, thing, &identifierLength, sizeof(unsigned char));
	free(thing);
	if(!result) {
		free(identifier);
		free(identifierThing);

		rendererSaveProgramFail();
	}

	result = fileWrite(f, identifierThing, identifier, identifierLength);
	free(identifier);
	free(identifierThing);
	if(!result) rendererSaveProgramFail();

	GLint size;
	glGetProgramiv(ren->program, GL_PROGRAM_BINARY_LENGTH, &size);

	thing = memString(thingString, "%s data %s \"%s\"", programBinary,
					  forRenderer, ren->name, NULL);
	if(!thing) rendererSaveProgramFail();

	void *data = memAlloc(thing, size);
	free(thing);
	if(!data) rendererSaveProgramFail();

	GLenum format;
	glGetProgramBinary(ren->program, size, NULL, &format, data);

	thing = memString(thingString, "%s format %s \"%s\"", programBinary,
					  forRenderer, ren->name, NULL);
	if(!thing) {
		free(data);
		rendererSaveProgramFail();
	}

	result = fileWrite(f, thing, &format, sizeof(GLenum));
	free(thing);
	if(!result) {
		free(data);
		rendererSaveProgramFail();
	}

	thing = memString(thingString, "%s size %s \"%s\"", programBinary,
					  forRenderer, ren->name, NULL);
	if(!thing) {
		free(data);
		rendererSaveProgramFail();
	}

	result = fileWrite(f, thing, &size, sizeof(GLint));
	free(thing);
	if(!result) {
		free(data);
		rendererSaveProgramFail();
	}

	thing = memString(thingString, "%s %s \"%s\"", programBinary, forRenderer,
					  ren->name, NULL);
	if(!thing) {
		free(data);
		rendererSaveProgramFail();
	}

	result = fileWrite(f, thing, data, size);
	free(thing);
	free(data);
	if(!result) rendererSaveProgramFail();

	fileFree(f);

	return true;
}
