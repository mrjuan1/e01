#include "window.h"

#include <SDL2/SDL.h>

#include "config.h"
#include "log.h"

#define windowInitFail(thing)                                         \
	{                                                                 \
		err("%s\n%s%s.\n", SDL_GetError(), windowThingString, thing); \
		return false;                                                 \
	}
#define windowInitCheck(thing) \
	if(result) windowInitFail(thing)
#define windowGLAttributeCheck() windowInitCheck(thingString)

const char *windowThingString = "Failed to ";

struct {
	SDL_Window *window;
	SDL_GLContext context;

	bool running, active, resized;
	int width, height;

	int startTicks, lastTicks;
	float deltaTime;
} windowData;

bool windowSetGLAttributes();
void windowResize();

bool windowInit() {
	int result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	windowInitCheck("initialise");

#ifdef DEBUG
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif // DEBUG

	if(!windowSetGLAttributes()) return false;

	// debug("Creating window...\n", 0);
	const configWindowMode windowMode = configGetWindowMode();
	const int position = windowMode == cwm_normal	 ? SDL_WINDOWPOS_CENTERED :
						 windowMode == cwm_resizable ? SDL_WINDOWPOS_UNDEFINED :
														 0;
	const unsigned short width = windowMode == cwm_borderless ?
									   0 :
									   configGetWidth(),
						 height = windowMode == cwm_borderless ?
										0 :
										configGetHeight();
	Uint32 flags = SDL_WINDOW_SHOWN;
	if(windowMode == cwm_resizable)
		flags |= SDL_WINDOW_RESIZABLE;
	else if(windowMode == cwm_fullscreen || windowMode == cwm_borderless) {
		flags |= SDL_WINDOW_BORDERLESS;

		if(windowMode == cwm_fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN;
		else
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	flags |= SDL_WINDOW_OPENGL;
	windowData.window = SDL_CreateWindow(configGetTitle(), position, position,
										 width, height, flags);
	if(!windowData.window) windowInitFail("create window");

	const configCursorMode cursorMode = configGetCursorMode();
	if(cursorMode != ccm_normal) {
		SDL_ShowCursor(SDL_DISABLE);
		if(cursorMode == ccm_captured) SDL_SetRelativeMouseMode(SDL_TRUE);
	}

	// debug("Creating context...\n", 0);
	windowData.context = SDL_GL_CreateContext(windowData.window);
	if(!windowData.context) windowInitFail("create context");

	if(configGetVsync() && SDL_GL_SetSwapInterval(-1) == -1) {
		const char *thingString = "Failed to set swap interval";

		debug("%s\n%s to -1, attempting to set it to 1...\n", SDL_GetError(),
			  thingString);

		if(SDL_GL_SetSwapInterval(1) == -1)
			warn("%s\nFailed to set swap interval, vsync not available.\n",
				 SDL_GetError(), thingString);
		else
			debug("Swap interval set to 1, vsync available.\n", 0);
	}

	windowResize();

	return (windowData.running = windowData.active = true);
}

void windowFree() {
	if(windowData.context) SDL_GL_DeleteContext(windowData.context);
	if(windowData.window) SDL_DestroyWindow(windowData.window);

	debug("End reached.\n", 0);
	SDL_Quit();
}

bool windowRunning() {
	SDL_Event event;

	while(SDL_PollEvent(&event)) //
		switch(event.type) {
			case SDL_WINDOWEVENT:
				switch(event.window.event) {
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						if(configGetCursorMode() == ccm_hidden ||
						   configGetCursorMode() == ccm_captured) {
							SDL_ShowCursor(SDL_DISABLE);

							if(configGetCursorMode() == ccm_captured)
								SDL_SetRelativeMouseMode(SDL_TRUE);
						}

						windowData.active = true;

						break;

					case SDL_WINDOWEVENT_FOCUS_LOST:
						windowData.active = false;

						if(configGetCursorMode() == ccm_hidden ||
						   configGetCursorMode() == ccm_captured) {
							if(configGetCursorMode() == ccm_captured)
								SDL_SetRelativeMouseMode(SDL_FALSE);

							SDL_ShowCursor(SDL_ENABLE);
						}

						break;

					case SDL_WINDOWEVENT_RESIZED: windowResize();
				}

				break;

				// Handle input

			case SDL_QUIT: windowData.running = false;
		}

	if(windowData.active) {
		const int ticks = SDL_GetTicks() - windowData.startTicks;
		windowData.deltaTime = (float)(ticks - windowData.lastTicks) / 1000.0f;
		windowData.lastTicks = ticks;
	}

	return windowData.running;
}

bool windowActive() { return windowData.active; }

void windowUpdate() {
	if(windowData.active) {
		SDL_GL_SwapWindow(windowData.window);
		SDL_Delay(10);

		windowData.resized = false;
	} else {
		SDL_Delay(500);
		windowResetTicks();
	}
}

bool windowResized() { return windowData.resized; }

int windowWidth() { return windowData.width; }

int windowHeight() { return windowData.height; }

void windowResetTicks() {
	windowData.startTicks = SDL_GetTicks();
	windowData.lastTicks = 0;
	windowData.deltaTime = 0.0f;
}

float windowDeltaTime() { return windowData.deltaTime; }

void windowQuit() { windowData.running = false; }

bool windowSetGLAttributes() {
	// debug("Setting GL attributes...\n", 0);

	const char *thingString = "set GL attribute";

	int result = SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 24);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	windowGLAttributeCheck();

	result = SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
	windowGLAttributeCheck();

	const bool vsync = configGetVsync();
	result = SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, vsync);
	windowGLAttributeCheck();

	return true;
}

void windowResize() {
	SDL_GL_GetDrawableSize(windowData.window, &windowData.width,
						   &windowData.height);
	windowData.resized = true;

	// debug("Size: %ix%i\n", windowData.width, windowData.height);
}
