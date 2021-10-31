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

unsigned char configGetSamples();
bool configGetVsync();
configWindowMode configGetWindowMode();
unsigned short configGetWidth();
unsigned short configGetHeight();
const char *configGetTitle();
configCursorMode configGetCursorMode();

void configSetSamples(unsigned char samples);
void configSetVsync(bool vsync);
void configSetWindowMode(configWindowMode windowMode);
void configSetWidth(unsigned short width);
void configSetHeight(unsigned short height);

bool configSave();

#endif // __CONFIG_H__
