#include "config.h"

#include <malloc.h>
#include <string.h>

#include "file.h"
#include "log.h"
#include "mem.h"

#define CONFIG_FILENAME "config.bin"

#define CONFIG_DEFAULT_SAMPLES 8
#define CONFIG_DEFAULT_VSYNC true

#ifdef DEBUG
#define CONFIG_DEFAULT_WINDOW_MODE cwm_resizable
#define CONFIG_DEFAULT_WIDTH 1280
#define CONFIG_DEFAULT_HEIGHT 720
#else // DEBUG
#define CONFIG_DEFAULT_WINDOW_MODE cwm_borderless
#define CONFIG_DEFAULT_WIDTH 0
#define CONFIG_DEFAULT_HEIGHT 0
#endif // DEBUG

#define CONFIG_DEFAULT_TITLE "E01"

#ifdef DEBUG
#define CONFIG_DEFAULT_CURSOR_MODE ccm_normal
#else // DEBUG
#define CONFIG_DEFAULT_CURSOR_MODE ccm_hidden
#endif // DEBUG

#define configSaveDefaults()                                              \
	{                                                                     \
		info("Config could not be loaded, attempting to create...\n", 0); \
		return configSetDefaults() && configSave();                       \
	}
#define configInitFail()      \
	{                         \
		fileFree(f);          \
		configSaveDefaults(); \
	}
#define configSaveFail() \
	{                    \
		fileFree(f);     \
		return false;    \
	}

struct {
	unsigned char samples;
	bool vsync;

	configWindowMode windowMode;
	unsigned short width, height;
	char *title;

	configCursorMode cursorMode;
} configData;

bool configSetDefaults();

bool configInit() {
	debug("Loading config \"%s\"...\n", CONFIG_FILENAME);

	file *f = fileInit(CONFIG_FILENAME, fm_read);
	if(!f) configSaveDefaults();

	memset(&configData, 0, sizeof(configData));

	if(!fileRead(f, "samples", &configData.samples, sizeof(unsigned char)))
		configInitFail();

	if(!fileRead(f, "vsync", &configData.vsync, sizeof(bool))) configInitFail();

	if(!fileRead(f, "window mode", &configData.windowMode,
				 sizeof(configWindowMode)))
		configInitFail();

	if(!fileRead(f, "width", &configData.width, sizeof(unsigned short)))
		configInitFail();

	if(!fileRead(f, "height", &configData.height, sizeof(unsigned short)))
		configInitFail();

	unsigned char titleLength;
	if(!fileRead(f, "title length", &titleLength, sizeof(unsigned char)))
		configInitFail();

	configData.title = fileReadAlloc(f, "title", titleLength);
	if(!configData.title) configInitFail();

	if(!fileRead(f, "cursor mode", &configData.cursorMode,
				 sizeof(configCursorMode)))
		configInitFail();

	fileFree(f);

	return true;
}

void configFree() {
	if(configData.title) free(configData.title);
	memset(&configData, 0, sizeof(configData));
}

// Gets
unsigned char configGetSamples() { return configData.samples; }

bool configGetVsync() { return configData.vsync; }

configWindowMode configGetWindowMode() { return configData.windowMode; }

unsigned short configGetWidth() { return configData.width; }

unsigned short configGetHeight() { return configData.height; }

const char *configGetTitle() { return configData.title; }

configCursorMode configGetCursorMode() { return configData.cursorMode; }

// Sets
void configSetSamples(unsigned char samples) { configData.samples = samples; }

void configSetVsync(bool vsync) { configData.vsync = vsync; }

void configSetWindowMode(configWindowMode windowMode) {
	configData.windowMode = windowMode;
}

void configSetWidth(unsigned short width) { configData.width = width; }

void configSetHeight(unsigned short height) { configData.height = height; }

// Save
bool configSave() {
	debug("Saving config \"%s\"...\n", CONFIG_FILENAME);

	file *f = fileInit(CONFIG_FILENAME, fm_write);
	if(!f) return false;

	if(!fileWrite(f, "samples", &configData.samples, sizeof(unsigned char)))
		configSaveFail();

	if(!fileWrite(f, "vsync", &configData.vsync, sizeof(bool)))
		configSaveFail();

	if(!fileWrite(f, "window mode", &configData.windowMode,
				  sizeof(configWindowMode)))
		configSaveFail();

	if(!fileWrite(f, "width", &configData.width, sizeof(unsigned short)))
		configSaveFail();

	if(!fileWrite(f, "height", &configData.height, sizeof(unsigned short)))
		configSaveFail();

	const unsigned char titleLength = strlen(configData.title) + 1;
	if(!fileWrite(f, "title length", &titleLength, sizeof(unsigned char)))
		configSaveFail();

	if(!fileWrite(f, "title", configData.title, titleLength)) configSaveFail();

	if(!fileWrite(f, "cursor mode", &configData.cursorMode,
				  sizeof(configCursorMode)))
		configSaveFail();

	fileFree(f);

	info("Config saved.\n", 0);

	return true;
}

bool configSetDefaults() {
	debug("Setting config defaults...\n", 0);

	memset(&configData, 0, sizeof(configData));

	configData.samples = CONFIG_DEFAULT_SAMPLES;
	configData.vsync = CONFIG_DEFAULT_VSYNC;

	configData.windowMode = CONFIG_DEFAULT_WINDOW_MODE;
	configData.width = CONFIG_DEFAULT_WIDTH;
	configData.height = CONFIG_DEFAULT_HEIGHT;

	if(configData.title) free(configData.title);
	configData.title = memString("title", "%s", CONFIG_DEFAULT_TITLE, NULL);
	if(!configData.title) return false;

	configData.cursorMode = CONFIG_DEFAULT_CURSOR_MODE;

	return true;
}
