#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdbool.h>

typedef enum {
	cwm_normal,
	cwm_resizable,
	cwm_fullscreen,
	cwm_borderless
} configWindowMode;

typedef enum { //
	ccm_normal,
	ccm_hidden,
	ccm_captured
} configCursorMode;

bool configInit();
void configFree();

bool configGetVsync();
configWindowMode configGetWindowMode();
unsigned short configGetWidth();
unsigned short configGetHeight();
const char *configGetTitle();
configCursorMode configGetCursorMode();
unsigned char configGetSamples();
unsigned char configGetAnisotropy();

void configSetVsync(bool vsync);
void configSetWindowMode(configWindowMode windowMode);
void configSetWidth(unsigned short width);
void configSetHeight(unsigned short height);
void configSetSamples(unsigned char samples);
void configSetAnisotropy(unsigned char anisotropy);

bool configSave();

#endif // __CONFIG_H__
